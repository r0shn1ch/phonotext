#pragma once

#include <QMainWindow>
#include <QStringList>

class QListWidget;
class QPushButton;
class QProgressBar;
class QTextEdit;
class QComboBox;
class QDoubleSpinBox;
class QCheckBox;
class QLineEdit;
class QLabel;
class QThread;

class TextAnalyzer;

class QtMainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit QtMainWindow(QWidget* parent = nullptr);
    ~QtMainWindow() override;

private slots:
    void addFiles();
    void addFolder();
    void removeSelected();
    void clearList();

    void chooseOutputDir();

    void startAnalysis();
    void cancelAnalysis();

    void onProgress(int percent, const QString& stageText);
    void onLog(const QString& text);
    void onFileDone(const QString& inputPath, const QString& outTxt, const QString& outJson);
    void onFinished(bool ok, const QString& summary);

private:
    void buildUi();
    void reloadLanguageConfigs();
    void setUiEnabled(bool enabled);
    QStringList inputFiles() const;

private:
    QListWidget* m_fileList = nullptr;
    QPushButton* m_addFilesBtn = nullptr;
    QPushButton* m_addFolderBtn = nullptr;
    QPushButton* m_removeBtn = nullptr;
    QPushButton* m_clearBtn = nullptr;

    QComboBox* m_languageBox = nullptr;
    QDoubleSpinBox* m_minPower = nullptr;
    QDoubleSpinBox* m_maxPower = nullptr;
    QCheckBox* m_generateTxt = nullptr;
    QCheckBox* m_generateJson = nullptr;

    QLineEdit* m_outputDir = nullptr;
    QPushButton* m_chooseOutBtn = nullptr;

    QPushButton* m_startBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;

    QProgressBar* m_progress = nullptr;
    QLabel* m_stageLabel = nullptr;
    QTextEdit* m_log = nullptr;

    QThread* m_workerThread = nullptr;
    TextAnalyzer* m_worker = nullptr;
};
