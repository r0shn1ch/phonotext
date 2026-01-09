#include "qtmainwindow.h"

#include <QListWidget>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QThread>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "textanalyzer.h"

static QString defaultOutputDir()
{
    return QDir::homePath() + QDir::separator() + "PhonotextOutput";
}

QtMainWindow::QtMainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    buildUi();

    // Автопоиск конфигов языков
    reloadLanguageConfigs();

    m_outputDir->setText(defaultOutputDir());
    QDir().mkpath(m_outputDir->text());

    connect(m_addFilesBtn, &QPushButton::clicked, this, &QtMainWindow::addFiles);
    connect(m_addFolderBtn, &QPushButton::clicked, this, &QtMainWindow::addFolder);
    connect(m_removeBtn, &QPushButton::clicked, this, &QtMainWindow::removeSelected);
    connect(m_clearBtn, &QPushButton::clicked, this, &QtMainWindow::clearList);
    connect(m_chooseOutBtn, &QPushButton::clicked, this, &QtMainWindow::chooseOutputDir);
    connect(m_startBtn, &QPushButton::clicked, this, &QtMainWindow::startAnalysis);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QtMainWindow::cancelAnalysis);

    setWindowTitle("Phonotext — анализ текста");
    resize(980, 640);
    setUiEnabled(true);
}

QtMainWindow::~QtMainWindow()
{
    cancelAnalysis();
}

void QtMainWindow::buildUi()
{
    auto* central = new QWidget(this);
    setCentralWidget(central);

    m_fileList = new QListWidget(central);
    m_fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_addFilesBtn = new QPushButton("Добавить файлы…", central);
    m_addFolderBtn = new QPushButton("Добавить папку…", central);
    m_removeBtn = new QPushButton("Удалить выбранные", central);
    m_clearBtn = new QPushButton("Очистить список", central);

    auto* fileBtnRow = new QHBoxLayout();
    fileBtnRow->addWidget(m_addFilesBtn);
    fileBtnRow->addWidget(m_addFolderBtn);
    fileBtnRow->addWidget(m_removeBtn);
    fileBtnRow->addWidget(m_clearBtn);

    auto* leftCol = new QVBoxLayout();
    leftCol->addWidget(new QLabel("Входные файлы", central));
    leftCol->addWidget(m_fileList, 1);
    leftCol->addLayout(fileBtnRow);

    auto* optionsBox = new QGroupBox("Параметры", central);
    m_languageBox = new QComboBox(optionsBox);
    // Заполняется в reloadLanguageConfigs()

    m_minPower = new QDoubleSpinBox(optionsBox);
    m_minPower->setRange(-1e9, 1e9);
    m_minPower->setDecimals(4);
    m_minPower->setValue(0.0);

    m_maxPower = new QDoubleSpinBox(optionsBox);
    m_maxPower->setRange(-1e9, 1e9);
    m_maxPower->setDecimals(4);
    m_maxPower->setValue(100.0);

    m_generateTxt = new QCheckBox("Сохранять TXT-отчёт", optionsBox);
    m_generateTxt->setChecked(true);
    m_generateJson = new QCheckBox("Сохранять JSON", optionsBox);
    m_generateJson->setChecked(true);

    m_outputDir = new QLineEdit(optionsBox);
    m_chooseOutBtn = new QPushButton("Выбрать…", optionsBox);

    auto* outRow = new QHBoxLayout();
    outRow->addWidget(m_outputDir, 1);
    outRow->addWidget(m_chooseOutBtn);

    auto* form = new QFormLayout();
    form->addRow("Конфиг языка:", m_languageBox);
    form->addRow("Мин. сила:", m_minPower);
    form->addRow("Макс. сила:", m_maxPower);
    form->addRow("Папка вывода:", outRow);
    form->addRow(m_generateTxt);
    form->addRow(m_generateJson);
    optionsBox->setLayout(form);

    auto* progressBox = new QGroupBox("Прогресс", central);
    m_progress = new QProgressBar(progressBox);
    m_progress->setRange(0, 100);
    m_progress->setValue(0);
    m_stageLabel = new QLabel("Ожидание", progressBox);

    m_startBtn = new QPushButton("Старт", progressBox);
    m_cancelBtn = new QPushButton("Отмена", progressBox);

    auto* progBtns = new QHBoxLayout();
    progBtns->addWidget(m_startBtn);
    progBtns->addWidget(m_cancelBtn);

    auto* progLayout = new QVBoxLayout();
    progLayout->addWidget(m_stageLabel);
    progLayout->addWidget(m_progress);
    progLayout->addLayout(progBtns);
    progressBox->setLayout(progLayout);

    auto* logBox = new QGroupBox("Журнал", central);
    m_log = new QTextEdit(logBox);
    m_log->setReadOnly(true);
    auto* logLayout = new QVBoxLayout();
    logLayout->addWidget(m_log);
    logBox->setLayout(logLayout);

    auto* rightCol = new QVBoxLayout();
    rightCol->addWidget(optionsBox);
    rightCol->addWidget(progressBox);
    rightCol->addWidget(logBox, 1);

    auto* root = new QHBoxLayout(central);
    root->addLayout(leftCol, 1);
    root->addLayout(rightCol, 1);
}

void QtMainWindow::setUiEnabled(bool enabled)
{
    m_addFilesBtn->setEnabled(enabled);
    m_addFolderBtn->setEnabled(enabled);
    m_removeBtn->setEnabled(enabled);
    m_clearBtn->setEnabled(enabled);
    m_languageBox->setEnabled(enabled);
    m_minPower->setEnabled(enabled);
    m_maxPower->setEnabled(enabled);
    m_generateTxt->setEnabled(enabled);
    m_generateJson->setEnabled(enabled);
    m_outputDir->setEnabled(enabled);
    m_chooseOutBtn->setEnabled(enabled);
    m_startBtn->setEnabled(enabled);

    m_cancelBtn->setEnabled(!enabled);
}

void QtMainWindow::reloadLanguageConfigs()
{
    m_languageBox->clear();

    QStringList dirs;
    const QString appDir = QCoreApplication::applicationDirPath();
    dirs << QDir(appDir).filePath("res/lng_conf")
         << QDir(appDir).filePath("../res/lng_conf")
         << QDir(appDir).filePath("../../res/lng_conf")
         << QDir(QDir::currentPath()).filePath("res/lng_conf")
         << QDir(QDir::currentPath()).filePath("../res/lng_conf");
    dirs.removeDuplicates();

    QList<QString> jsonFiles;
    for (const QString& d : dirs)
    {
        QDir dir(d);
        if (!dir.exists())
            continue;
        const QFileInfoList list = dir.entryInfoList({"*.json"}, QDir::Files, QDir::Name);
        for (const QFileInfo& fi : list)
            jsonFiles.push_back(fi.absoluteFilePath());
    }

    std::sort(jsonFiles.begin(), jsonFiles.end());
    jsonFiles.erase(std::unique(jsonFiles.begin(), jsonFiles.end()), jsonFiles.end());

    int russianIndex = -1;

    for (const QString& path : jsonFiles)
    {
        QFile f(path);
        QString display;
        if (f.open(QIODevice::ReadOnly))
        {
            const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
            if (doc.isObject())
            {
                const QJsonObject obj = doc.object();
                const QJsonValue nameV = obj.value("name");
                if (nameV.isString())
                    display = nameV.toString().trimmed();
            }
            f.close();
        }
        if (display.isEmpty())
        {
            QFileInfo fi(path);
            display = fi.completeBaseName();
        }

        const int idx = m_languageBox->count();
        m_languageBox->addItem(display, path);

        const QString base = QFileInfo(path).fileName().toLower();
        if (base == "russian.json" || display.toLower().contains("рус"))
            russianIndex = idx;
    }

    if (m_languageBox->count() == 0)
    {
        m_languageBox->addItem("Русский (по умолчанию)", "rus");
        m_languageBox->addItem("English (default)", "eng");
        m_languageBox->addItem("Latin (default)", "lat");
        russianIndex = 0;
    }

    if (russianIndex >= 0)
        m_languageBox->setCurrentIndex(russianIndex);
    else
        m_languageBox->setCurrentIndex(0);
}

QStringList QtMainWindow::inputFiles() const
{
    QStringList paths;
    paths.reserve(m_fileList->count());
    for (int i = 0; i < m_fileList->count(); ++i)
        paths << m_fileList->item(i)->text();
    return paths;
}

void QtMainWindow::addFiles()
{
    const QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Выберите текстовые файлы",
        QDir::homePath(),
        "Текстовые файлы (*.txt *.text *.md);;Все файлы (*.*)"
    );

    for (const auto& f : files)
        m_fileList->addItem(f);
}

void QtMainWindow::addFolder()
{
    const QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку", QDir::homePath());
    if (dir.isEmpty())
        return;

    QDir d(dir);
    const QStringList files = d.entryList({"*.txt", "*.text", "*.md"}, QDir::Files);
    for (const auto& f : files)
        m_fileList->addItem(d.absoluteFilePath(f));
}

void QtMainWindow::removeSelected()
{
    const auto items = m_fileList->selectedItems();
    for (auto* item : items)
        delete m_fileList->takeItem(m_fileList->row(item));
}

void QtMainWindow::clearList()
{
    m_fileList->clear();
}

void QtMainWindow::chooseOutputDir()
{
    const QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку вывода", m_outputDir->text());
    if (!dir.isEmpty())
        m_outputDir->setText(dir);
}

void QtMainWindow::startAnalysis()
{
    if (m_fileList->count() == 0)
    {
        QMessageBox::information(this, "Phonotext", "Добавьте хотя бы один входной файл.");
        return;
    }

    const QString outDir = m_outputDir->text().trimmed();
    if (outDir.isEmpty())
    {
        QMessageBox::warning(this, "Phonotext", "Папка вывода не задана.");
        return;
    }

    QDir().mkpath(outDir);

    setUiEnabled(false);
    m_progress->setValue(0);
    m_stageLabel->setText("Запуск…");
    m_log->clear();

    m_workerThread = new QThread(this);
    m_worker = new TextAnalyzer();
    m_worker->moveToThread(m_workerThread);

    const QString lang = m_languageBox->currentData().toString();
    const double minP = m_minPower->value();
    const double maxP = m_maxPower->value();
    const bool genTxt = m_generateTxt->isChecked();
    const bool genJson = m_generateJson->isChecked();

    m_worker->setJob(inputFiles(), outDir, lang, minP, maxP, genTxt, genJson);

    connect(m_workerThread, &QThread::started, m_worker, &TextAnalyzer::run);
    connect(m_worker, &TextAnalyzer::progress, this, &QtMainWindow::onProgress);
    connect(m_worker, &TextAnalyzer::log, this, &QtMainWindow::onLog);
    connect(m_worker, &TextAnalyzer::fileDone, this, &QtMainWindow::onFileDone);
    connect(m_worker, &TextAnalyzer::finished, this, &QtMainWindow::onFinished);

    connect(m_worker, &TextAnalyzer::finished, m_workerThread, &QThread::quit);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);

    m_workerThread->start();
}

void QtMainWindow::cancelAnalysis()
{
    if (!m_worker)
        return;

    m_worker->requestCancel();
}

void QtMainWindow::onProgress(int percent, const QString& stageText)
{
    m_progress->setValue(percent);
    if (!stageText.isEmpty())
        m_stageLabel->setText(stageText);
}

void QtMainWindow::onLog(const QString& text)
{
    m_log->append(text);
}

void QtMainWindow::onFileDone(const QString& inputPath, const QString& outTxt, const QString& outJson)
{
    QString msg = QString("Done: %1\n  TXT: %2\n  JSON: %3")
                      .arg(inputPath)
                      .arg(outTxt.isEmpty() ? "(not generated)" : outTxt)
                      .arg(outJson.isEmpty() ? "(not generated)" : outJson);
    m_log->append(msg);
}

void QtMainWindow::onFinished(bool ok, const QString& summary)
{
    m_worker = nullptr;
    m_workerThread = nullptr;

    setUiEnabled(true);
    m_stageLabel->setText(ok ? "Готово" : "Остановлено");
    m_progress->setValue(ok ? 100 : m_progress->value());

    if (!summary.isEmpty())
        m_log->append(summary);
}
