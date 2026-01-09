#pragma once

#include <QObject>
#include <QStringList>
#include <atomic>

class AnalyzerWorker final : public QObject
{
    Q_OBJECT

public:
    explicit AnalyzerWorker(QObject* parent = nullptr);

    void setInputs(const QStringList& inputFiles);
    void setOutputDir(const QString& outputDir);
    void setLanguage(const QString& languageCode);
    void setPowerRange(double minPower, double maxPower);
    void setWriters(bool writeTxt, bool writeJson);

    // Convenience helper for UI code
    void setJob(const QStringList& inputFiles,
                const QString& outputDir,
                const QString& languageCode,
                double minPower,
                double maxPower,
                bool writeTxt,
                bool writeJson)
    {
        setInputs(inputFiles);
        setOutputDir(outputDir);
        setLanguage(languageCode);
        setPowerRange(minPower, maxPower);
        setWriters(writeTxt, writeJson);
    }

public slots:
    void run();
    void cancel();

    // Alias for readability
    void requestCancel() { cancel(); }

signals:
    void progress(int percent, const QString& stageText);
    void log(const QString& text);
    void fileDone(const QString& inputPath, const QString& outTxt, const QString& outJson);
    void finished(bool ok, const QString& summary);

private:
    QString outPathFor(const QString& inputPath, const QString& suffix) const;

private:
    QStringList m_inputs;
    QString m_outputDir;
    QString m_language;
    double m_minPower = 0.0;
    double m_maxPower = 100.0;
    bool m_writeTxt = true;
    bool m_writeJson = true;
    std::atomic_bool m_cancelled{false};
};
