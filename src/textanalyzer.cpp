#include "textanalyzer.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>
#include <QElapsedTimer>

#include <atomic>

#include "phonotext.h"
#include "proccessing.h"

TextAnalyzer::TextAnalyzer(QObject* parent)
    : QObject(parent)
{
}

void TextAnalyzer::setInputs(const QStringList& inputFiles)
{
    m_inputs = inputFiles;
}

void TextAnalyzer::setOutputDir(const QString& outputDir)
{
    m_outputDir = outputDir;
}

void TextAnalyzer::setLanguage(const QString& languageCode)
{
    m_language = languageCode;
}

void TextAnalyzer::setPowerRange(double minPower, double maxPower)
{
    m_minPower = minPower;
    m_maxPower = maxPower;
}

void TextAnalyzer::setWriters(bool writeTxt, bool writeJson)
{
    m_writeTxt = writeTxt;
    m_writeJson = writeJson;
}

QString TextAnalyzer::outPathFor(const QString& inputPath, const QString& suffix) const
{
    QFileInfo fi(inputPath);
    QString base = fi.completeBaseName();
    return QDir(m_outputDir).filePath(base + suffix);
}

void TextAnalyzer::cancel()
{
    m_cancelled.store(true, std::memory_order_relaxed);
}

void TextAnalyzer::run()
{
    m_cancelled.store(false, std::memory_order_relaxed);

    if (m_inputs.isEmpty())
    {
        emit finished(false, "Не выбраны входные файлы.");
        return;
    }

    if (m_outputDir.isEmpty())
    {
        emit finished(false, "Не задана папка вывода.");
        return;
    }

    QDir().mkpath(m_outputDir);

    QElapsedTimer total;
    total.start();

    const int fileCount = m_inputs.size();
    int filesOk = 0;

    for (int i = 0; i < fileCount; ++i)
    {
        if (m_cancelled.load(std::memory_order_relaxed))
        {
            emit log("Отменено пользователем.");
            emit finished(false, "Отменено.");
            return;
        }

        const QString inputPath = m_inputs[i];
        emit log(QString("Чтение: %1").arg(inputPath));
        emit progress((i * 100) / fileCount, "Чтение файла");

        QFile inFile(inputPath);
        if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            emit log(QString("ОШИБКА: не удалось открыть %1").arg(inputPath));
            continue;
        }

        QTextStream in(&inFile);
        const QString text = in.readAll();
        inFile.close();

        Phonotext pt(text.toStdString());

        const std::string lng = m_language.toStdString();

        const int fileBase = (i * 100) / fileCount;
        const int fileSpan = std::max(1, 100 / fileCount);

        std::atomic<int> lastPercent{fileBase};

        try
        {
            Proccessing proc(std::move(pt), lng, m_minPower, m_maxPower,
                             [&](int localPercent, const std::string& stage)
                             {
                                 int p = fileBase + (localPercent * fileSpan) / 100;
                                 int prev = lastPercent.load();
                                 if (p > prev)
                                 {
                                     lastPercent.store(p);
                                     emit progress(p, QString::fromStdString(stage));
                                 }
                             },
                             [&](const std::string& msg)
                             {
                                 emit log(QString::fromStdString(msg));
                             });

            QString outTxt;
            QString outJson;

            if (m_writeTxt)
            {
                outTxt = outPathFor(inputPath, "_out.txt");
                proc.print(outTxt);
            }
            if (m_writeJson)
            {
                outJson = outPathFor(inputPath, "_out.json");
                proc.createJson(outJson.toStdString());
            }

            emit fileDone(inputPath, outTxt, outJson);
            emit log(QString("Готово: %1").arg(inputPath));
            ++filesOk;
        }
        catch (const std::exception& e)
        {
            emit log(QString("ОШИБКА: %1").arg(e.what()));
            continue;
        }
    }

    emit progress(100, "Завершено");
    emit finished(true, QString("Обработано %1/%2 файлов за %3 мс")
                          .arg(filesOk)
                          .arg(fileCount)
                          .arg(total.elapsed()));
}
