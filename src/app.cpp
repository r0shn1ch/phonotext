#include "app.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>
#include <QElapsedTimer>

#include <atomic>

#include "phonotext.h"
#include "proccessing.h"

AnalyzerWorker::AnalyzerWorker(QObject* parent)
    : QObject(parent)
{
}

void AnalyzerWorker::setInputs(const QStringList& inputFiles)
{
    m_inputs = inputFiles;
}

void AnalyzerWorker::setOutputDir(const QString& outputDir)
{
    m_outputDir = outputDir;
}

void AnalyzerWorker::setLanguage(const QString& languageCode)
{
    m_language = languageCode;
}

void AnalyzerWorker::setPowerRange(double minPower, double maxPower)
{
    m_minPower = minPower;
    m_maxPower = maxPower;
}

void AnalyzerWorker::setWriters(bool writeTxt, bool writeJson)
{
    m_writeTxt = writeTxt;
    m_writeJson = writeJson;
}

QString AnalyzerWorker::outPathFor(const QString& inputPath, const QString& suffix) const
{
    QFileInfo fi(inputPath);
    QString base = fi.completeBaseName();
    return QDir(m_outputDir).filePath(base + suffix);
}

void AnalyzerWorker::cancel()
{
    m_cancelled.store(true, std::memory_order_relaxed);
}

void AnalyzerWorker::run()
{
    m_cancelled.store(false, std::memory_order_relaxed);

    if (m_inputs.isEmpty())
    {
        emit finished(false, "No input files selected.");
        return;
    }

    if (m_outputDir.isEmpty())
    {
        emit finished(false, "Output directory is empty.");
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
            emit log("Cancelled by user.");
            emit finished(false, "Cancelled.");
            return;
        }

        const QString inputPath = m_inputs[i];
        emit log(QString("Reading: %1").arg(inputPath));
        emit progress((i * 100) / fileCount, "Reading file");

        QFile inFile(inputPath);
        if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            emit log(QString("ERROR: Cannot open %1").arg(inputPath));
            continue;
        }

        QTextStream in(&inFile);
        const QString text = in.readAll();
        inFile.close();

        Phonotext pt(text.toStdString());

        // Map UI language selection to existing config names
        const std::string lng = m_language.toStdString();

        const int fileBase = (i * 100) / fileCount;
        const int fileSpan = std::max(1, 100 / fileCount);

        std::atomic<int> lastPercent{fileBase};

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
        emit log(QString("Done: %1").arg(inputPath));
        ++filesOk;
    }

    emit progress(100, "Finished");
    emit finished(true, QString("Processed %1/%2 files in %3 ms")
                          .arg(filesOk)
                          .arg(fileCount)
                          .arg(total.elapsed()));
}
