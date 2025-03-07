﻿#include <string.h>

#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QStandardPaths>
#include <QMessageBox>
#include <QColorDialog>
#include <QSettings>
#include <QButtonGroup>
#include <QListView>
#include <QFileDialog>
#include <QTimer>
#include <QFile>

#include "YString.h"
#include "YJson.h"
#include "dialog.h"
#include "ui_dialog.h"
#include "form.h"
#include "blankform.h"
#include "gmpoperatetip.h"
#include "dialogwallpaper.h"
#include "bingsetting.h"
#include "formsetting.h"
#include "desktopmask.h"

constexpr const char *color_theme[8] =
{
    "255,255,255,255",
    "255,255,255,150",
    "155,89,182,150",
    "255,10,10,150",
    "50,200,50,150",
    "135,160,250,150",
    "200,100,100,150",
    "0,0,0,100"
};

constexpr const char *reg_keys[4] = {
    "HKEY_CURRENT_USER\\SOFTWARE\\Classes\\*\\shell\\QCoper",
    "HKEY_CURRENT_USER\\SOFTWARE\\Classes\\Directory\\shell\\QCoper",
    "HKEY_CURRENT_USER\\SOFTWARE\\Classes\\Directory\\Background\\shell\\QCoper",
    "mshta vbscript:clipboarddata.setdata(\"text\",\"%%1\")(close)"
};

inline QString getStyleSheet()
{
    QColor col(VarBox->dAlphaColor[VarBox->setMax] & 0xffffff);
    return QString("QPushButton{background-color: rgb(%1, %2, %3);border-radius: 3px; border: 1px solid black;}").arg(QString::number(col.blue()), QString::number(col.green()), QString::number(col.red()));
}


Dialog::Dialog():
    SpeedWidget<QWidget>(nullptr),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);                                   // 一定要在下面这句之前，否则会出现问题。
    initChildren();
    initUi();
	initConnects();
    initButtonFilter();
}

Dialog::~Dialog()
{
    qout << "析构dialog开始";
    delete buttonGroup;
    delete ui;
    qout << "析构dialog结束";
}

void Dialog::initUi()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);  //| Qt::WindowStaysOnTopHint
    setAttribute(Qt::WA_DeleteOnClose, true);
    if (VarBox->WinVersion != 0xA)
    {
        ui->radioButton_7->setEnabled(false);
        ui->radioButton_8->setEnabled(false);
        ui->radioButton_9->setEnabled(false);
    }

    QFile qss(":/qss/dialog_style.qss");
    qss.open(QFile::ReadOnly);
    setStyleSheet(QString(qss.readAll()));
    qss.close();
    initSpeedBox(ui->frame, &Dialog::showMinimized, &Dialog::close);
    buttonGroup->addButton(ui->rBtnNew, (int)PAPER_TYPE::Latest);                //将按钮添加到按钮组合，同时设置按钮 id
    buttonGroup->addButton(ui->rBtnHot, (int)PAPER_TYPE::Hot);
    buttonGroup->addButton(ui->rBtnNature, (int)PAPER_TYPE::Nature);
    buttonGroup->addButton(ui->rBtnAnime, (int)PAPER_TYPE::Anime);
    buttonGroup->addButton(ui->rBtnSimple, (int)PAPER_TYPE::Simple);
    buttonGroup->addButton(ui->rBtnRandom, (int)PAPER_TYPE::Random);
    buttonGroup->addButton(ui->rBtnBing, (int)PAPER_TYPE::Bing);
    buttonGroup->addButton(ui->rBtnNative, (int)PAPER_TYPE::Native);
    buttonGroup->addButton(ui->rBtnAdvance, (int)PAPER_TYPE::Advance);
	buttonGroup->setExclusive(true);                     // 按钮之间相互排斥
    ui->lineAppData->setText(VarBox->get_dat_path());

    ui->checkBox_2->setChecked(false);
    ui->LinePath->setText(VarBox->NativeDir);
    ui->label_17->setText(QString::number(VarBox->RefreshTime));
    ui->label_4->setText(QString::number(VarBox->dAlphaColor[0] >> 24));
    ui->label_6->setText(QString::number(VarBox->bAlpha[0]));
    ui->sLdPageNum->setValue(VarBox->PageNum);
    ui->sLdTimeInterval->setValue(VarBox->TimeInterval);
    ui->labTimeInterval->setText(QString::number(VarBox->TimeInterval));
    ui->chkEnableChangePaper->setChecked(VarBox->AutoChange);
    ui->usrCmd->setText(VarBox->UserCommand);
    ui->linePictuerPath->setText(VarBox->MajorDir);
    ui->rBtnWinTaskMax->setChecked(VarBox->setMax);
    ui->sLdTaskAlph->setValue(VarBox->dAlphaColor[VarBox->setMax] >> 24);
    ui->sLdIconAlph->setValue(VarBox->bAlpha[VarBox->setMax]);
    ui->sLdUpdateRefreshTime->setValue(VarBox->RefreshTime);
    ui->pushButton_3->setStyleSheet(getStyleSheet());
    ui->lineNewName->setText(VarBox->CustomNames[ui->cBxstandardName->currentIndex()]);
    ui->line_APP_ID->setText(VarBox->AppId);
    ui->line_PASS_WORD->setText(VarBox->PassWord);
    ui->cBxAutoStart->setChecked(!QSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat).value("SpeedBox").toString().compare(qApp->applicationFilePath().replace("/", "\\")));
    switch (VarBox->aMode[VarBox->setMax])
    {
    case (ACCENT_STATE::ACCENT_DISABLED):                       // 默认
        ui->radioButton_3->setChecked(true);
        break;
    case (ACCENT_STATE::ACCENT_ENABLE_TRANSPARENTGRADIENT):     // 透明
        ui->radioButton_4->setChecked(true);
        break;
    case (ACCENT_STATE::ACCENT_ENABLE_BLURBEHIND):              // 玻璃
        ui->radioButton_5->setChecked(true);
        break;
    case (ACCENT_STATE::ACCENT_ENABLE_ACRYLICBLURBEHIND):       // 亚克力
        ui->radioButton_6->setChecked(true);
        break;
    default:
        break;
    }
    if (QSettings(TASK_DESK_SUB, QSettings::NativeFormat).contains(TASKBAR_ACRYLIC_OPACITY))
        switch (QSettings(TASK_DESK_SUB, QSettings::NativeFormat).value(TASKBAR_ACRYLIC_OPACITY).toInt())
        {
        case 10:                                              // 不透明
            ui->radioButton_11->setChecked(true);
            break;
        case 5:                                               // 半透明
            ui->radioButton_10->setChecked(true);
            break;
        case 0:                                               // 全透明
            ui->radioButton_12->setChecked(true);
            break;
        default:                                              // 默认不透明
            ui->radioButton_11->setChecked(true);
            break;
        }
    else
        ui->radioButton_11->setChecked(true);                 // 默认不透明
    switch (VarBox->iPos)
    {
    case TaskBarCenterState::TASK_LEFT:
        ui->radioButton_7->setChecked(true);
        break;
    case TaskBarCenterState::TASK_CENTER:
        ui->radioButton_8->setChecked(true);
        break;
    case TaskBarCenterState::TASK_RIGHT:
        ui->radioButton_9->setChecked(true);
        break;
    default:
        break;
    }
    if (VarBox->PathToOpen.compare(qApp->applicationDirPath().replace("/", "\\")))
        ui->radioButton_14->setChecked(true);
    else
        ui->radioButton_13->setChecked(true);
    ui->pushButton_4->setText("确定");
    ui->comboBox_3->setCurrentIndex((int)VarBox->CurTheme);
    ui->lineEdit->setText(VarBox->PathToOpen);
    ui->checkBox_3->setChecked(VarBox->FirstChange);
    ui->BtnChooseFolder->setEnabled(ui->rBtnNative->isChecked());
    ui->checkBox_4->setChecked(VarBox->ControlDesktopIcon);
    ui->frame->setStyleSheet(QString("QFrame{background-color:rgba(%1);}QLabel{border-radius: 3px;background-color: transparent;}Line{background-color:black};").arg(color_theme[static_cast<int>(VarBox->CurTheme)]));
    checkSettings();
    move((VarBox->ScreenWidth - width()) / 2, (VarBox->ScreenHeight - height()) / 2);
}

void Dialog::initChildren()
{
	buttonGroup = new QButtonGroup;
}

void Dialog::initConnects()
{
    connect(ui->rBtnNative, &QRadioButton::toggled, this, [this](bool checked){ui->BtnChooseFolder->setEnabled(checked);});
    connect(ui->BtnChooseFolder, &QToolButton::clicked, this, &Dialog::chooseFolder);
    connect(ui->pBtnCancel, &QPushButton::clicked, this, &Dialog::close);
    connect(ui->pBtnOk, &QPushButton::clicked, this, &Dialog::saveWallpaperSettings);
    connect(ui->pBtnOpenAppData, &QPushButton::clicked, [](){VARBOX::runCommand("explorer", QStringList(VarBox->get_dat_path()));});
    connect(ui->pBtnOpenPicturePath, &QPushButton::clicked, this, &Dialog::openPicturePath);
    connect(ui->linePictuerPath, &QLineEdit::returnPressed, this, &Dialog::linePictuerPathReturn);
    connect(ui->pBtnSaveTaskSettings, &QPushButton::clicked, [this](){VarBox->saveTrayStyle(); jobTip->showTip("保存成功！");});
    connect(ui->cBxAutoStart, &QPushButton::clicked, [this](bool checked){
        QSettings* reg = new QSettings(
            "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
            QSettings::NativeFormat);
        if (checked)
            reg->setValue("SpeedBox", qApp->applicationFilePath().replace("/", "\\"));
        else
            reg->remove("SpeedBox");
        delete reg;
        jobTip->showTip("修改成功！");
    });
    connect(ui->tBtnSetBing, &QToolButton::clicked, this, [this](){
        BingSetting x;
        x.move(frameGeometry().x()+(width()-x.width())/2, frameGeometry().y()+(height()-x.height())/2);
        x.exec();
    });
    connect(ui->pBtnCustomName, &QPushButton::clicked, this, &Dialog::setCustomName);
    connect(ui->lineNewName, &QLineEdit::returnPressed, this, &Dialog::setCustomName);
    connect(ui->pBtnApply, &QPushButton::clicked, this, &Dialog::applyWallpaperSettings);
    connect(ui->cBxstandardName, &QComboBox::currentIndexChanged, this, &Dialog::cBxstandardNameCurIndex);
    connect(ui->sLdPageNum, &QSlider::valueChanged, this, &Dialog::sLdPageNumCurNum);
    connect(ui->sLdUpdateRefreshTime, &QSlider::valueChanged, this, &Dialog::sLdUpdateTimeCurNum);
    connect(ui->sLdTaskAlph, &QSlider::valueChanged, this, &Dialog::sLdTaskAlphCurNum);
    connect(ui->sLdIconAlph, &QSlider::valueChanged, this, &Dialog::sLdIconAlphCurNum);
    connect(ui->sLdTimeInterval, &QSlider::valueChanged, this, [this](int value){ui->labTimeInterval->setText(QString::number(value, 10));});
    connect(ui->rBtnWinTaskNormal, &QRadioButton::clicked, [this](){changeType(false);});
    connect(ui->rBtnWinTaskMax, &QRadioButton::clicked, [this](){changeType(true);});
}

void Dialog::initButtonFilter()
{
    QList<QSlider *> sliders = findChildren<QSlider *>();
    QList<QSlider *>::iterator sliderList = sliders.begin();
    for (sliderList = sliders.begin(); sliderList != sliders.end(); sliderList++)
    {
        (*sliderList)->installEventFilter(this);
    }
    QList<QComboBox *> combs = findChildren<QComboBox *>();
    QList<QComboBox *>::iterator combList = combs.begin();
    for (combList = combs.begin(); combList != combs.end(); combList++)
    {
        (*combList)->installEventFilter(this);
        (*combList)->setView(new QListView());
    }
}

bool Dialog::eventFilter(QObject* target, QEvent* event)
{
    if (/*typeid (*target) == typeid (QPushButton) || */typeid (*target) == typeid (QComboBox))
    {
        if ((QEvent::Type)(event->type()) == QEvent::MouseMove)
            return true;
    }
    else if (typeid (*target) == typeid (QSlider))
    {
        QMouseEvent *env = static_cast<QMouseEvent *>(event);
        QSlider *tar = static_cast<QSlider *>(target);
        //static bool clicked = false;
        switch ((QEvent::Type)(env->type()))
        {
        case QEvent::MouseMove:
            tar->setValue(((double)env->pos().x() / (double)tar->width() * (tar->maximum() - tar->minimum())) + tar->minimum());
            return true;
        case QEvent::MouseButtonPress:
            tar->setValue(((double)env->pos().x() / (double)tar->width() * (tar->maximum() - tar->minimum())) + tar->minimum());
            tar->setMouseTracking(true);
            return true;
        case QEvent::MouseButtonRelease:
            tar->setMouseTracking(false);
            return true;
        default:
            break;
        }
    }
    return QWidget::eventFilter(target, event);
}

void Dialog::closeEvent(QCloseEvent *event)
{
    *const_cast<Dialog**>(&(VarBox->dialog)) = nullptr;
    event->accept();
}

/**
 * @brief Dialog::on_rBtnNative_toggled
 *
 * @当“本地”按钮被选中时，选择文件夹按钮可用。
 *
 * @param {checked} 按钮选中状态
 *
 */

void Dialog::chooseFolder()
{
	QDir d;
    if (VarBox->NativeDir.isEmpty() || (!d.exists(VarBox->NativeDir) && !d.mkdir(VarBox->NativeDir)))
	{
        VarBox->NativeDir = QDir::toNativeSeparators(QStandardPaths::writableLocation((QStandardPaths::PicturesLocation)));
        VarBox->sigleSave("Wallpaper", "NativeDir", VarBox->NativeDir);
        ui->LinePath->setText(VarBox->NativeDir);
	}
	QString titile = "请选择一个文件夹";
    QString dir = QFileDialog::getExistingDirectory(NULL, titile, VarBox->NativeDir, QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty())
        ui->LinePath->setText(QDir::toNativeSeparators(dir));
}

void Dialog::sLdPageNumCurNum(int value)
{
	ui->label_2->setText(QString::number(value, 10));
}

void Dialog::saveWallpaperSettings()
{
    QSettings IniWrite(VarBox->get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup("Wallpaper");
    IniWrite.setValue("PaperType", buttonGroup->checkedId());
    IniWrite.setValue("NativeDir", ui->LinePath->text());
    IniWrite.setValue("PageNum", ui->sLdPageNum->value());
    IniWrite.setValue("TimeInerval", ui->sLdTimeInterval->value());
    IniWrite.setValue("AutoChange", ui->chkEnableChangePaper->isChecked());
    IniWrite.setValue("UserCommand", ui->usrCmd->text());
    IniWrite.setValue("FirstChange", ui->checkBox_3->isChecked());
    IniWrite.endGroup();
    jobTip->showTip("保存成功！");
}

/**
 * @brief Dialog::on_pBtnApply_clicked
 *
 * @“应用”按钮被点击后，将界面上的配置导入到文件中，将文件中的数据加载到静态变量。
 *
 */

void Dialog::applyWallpaperSettings()
{
    Wallpaper::update = ui->checkBox_2->isChecked();
    VarBox->PaperType = (PAPER_TYPE)buttonGroup->checkedId();
    VarBox->NativeDir = ui->LinePath->text();
    VarBox->PageNum = ui->sLdPageNum->value();
    VarBox->TimeInterval = ui->sLdTimeInterval->value();
    VarBox->AutoChange = ui->chkEnableChangePaper->isChecked();
    VarBox->UserCommand = ui->usrCmd->text();
    VarBox->FirstChange = ui->checkBox_3->isChecked();
    for (int type = 0; type < 2; ++type)
    {
        if (type == 0)
        {
            if (VarBox->dwallpaper->isActive())
            {
                if (QMessageBox::question(this, "警告", "您之前的壁纸类型更换正在生效中，是否终止？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                {
                    setEnabled(false);
                    VarBox->RunApp = false;
                    VarBox->dwallpaper->clean();
                    VarBox->RunApp = true;
                    setEnabled(true);
                    if (QMessageBox::question(this, "提示", "终止成功！是否继续？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                        continue;
                }
                break;
            }
            else
            {
                if (Wallpaper::canCreat())
                    continue;
                QMessageBox::information(this, "提示", "右键更换壁纸正在运行，请稍后再试。", QMessageBox::Ok);
                break;
            }
        }
        else if (type == 1)
        {
            if (VarBox->AutoChange)
            {
                Wallpaper::initSet = true;
                if (VarBox->PaperType == PAPER_TYPE::Bing)
                    Wallpaper::bing = 0;
                VarBox->dwallpaper->iniStart();
                VarBox->change_paper_timer->setInterval(VarBox->TimeInterval * 60000);  // 设置时间间隔,Timer的单位是毫秒
                if (!VarBox->change_paper_timer->isActive()) VarBox->change_paper_timer->start();
                qout << "开始更换壁纸";
            }
            else
                if (VarBox->change_paper_timer->isActive()) VarBox->change_paper_timer->stop();
            if (QMessageBox::question(this, "提示", "应用成功！是否保存设置？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                saveWallpaperSettings();
        }
    }
}

void Dialog::checkSettings()
{
    ui->chkFile->setChecked(QSettings(reg_keys[0], QSettings::NativeFormat).contains("."));
    ui->chkFolder->setChecked(QSettings(reg_keys[1], QSettings::NativeFormat).contains("."));
    ui->chkFolderBack->setChecked(QSettings(reg_keys[2], QSettings::NativeFormat).contains("."));
    ui->chkTimeUnit_sec->setChecked(QSettings(TASK_DESK_SUB, QSettings::NativeFormat).contains(SHOW_SECONDS_IN_SYSTEM_CLOCK));

}

void Dialog::on_pBtnApply_2_clicked()
{
    static QString icon_path = VarBox->get_dat_path() + "\\Copy.ico";
	QFile file(icon_path);
	if (!file.exists() && file.open(QIODevice::WriteOnly))
	{
        QFile qss(":/icons/copy.ico");
		qss.open(QFile::ReadOnly);
		file.write(qss.readAll());
		qss.close();
		file.close();
	}
    const QCheckBox* box[3] = {ui->chkFile, ui->chkFolder, ui->chkFolderBack}; short type = -1;
    while (++type != 3)
    {
        if (box[type]->isChecked())
        {
            qout << "创建键:" << type;
            QSettings settings(reg_keys[type], QSettings::NativeFormat);
            if (!settings.contains("."))
            {
                settings.setValue(".", QString("复制路径"));
                settings.setValue("Icon", icon_path);
                settings.beginGroup("command");
                settings.setValue(".", QString(reg_keys[3]).arg(type==2?'V':'1'));
                settings.endGroup();
            }
        }
        else
        {
            qout << "删除键:" << type;
            QSettings settings(reg_keys[type], QSettings::NativeFormat);
            if (settings.contains("."))
            {
//                if (settings.contains("command"))
//                    settings.remove("command");
                settings.clear();
            }
        }
    }
    jobTip->showTip("设置成功！");
}

void Dialog::openPicturePath()
{
	QString str = ui->linePictuerPath->text();
	QDir dir;
    if (dir.exists(str) || (!str.isEmpty() && dir.mkdir(str)))
        VarBox->MajorDir = str.replace("/", "\\");
	else
	{
        if (dir.exists(VarBox->MajorDir) || dir.mkdir(VarBox->MajorDir))
            ui->linePictuerPath->setText(VarBox->MajorDir.replace("/", "\\"));
		else
            VarBox->MajorDir = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
	}
    VarBox->get_wal_path();
    if (dir.exists(VarBox->MajorDir))
        VARBOX::runCommand("explorer", QStringList(VarBox->MajorDir));
	else
		QMessageBox::warning(this, "警告", "路径不存在！", QMessageBox::Ok, QMessageBox::Ok);
}


void Dialog::linePictuerPathReturn()
{
	QDir dir;
	if (dir.exists(ui->linePictuerPath->text()))
	{
        VarBox->MajorDir = ui->linePictuerPath->text();
        VarBox->get_wal_path();
	}
	else
	{
		QMessageBox::warning(this, "警告", "路径不存在！", QMessageBox::Ok, QMessageBox::Ok);
        ui->linePictuerPath->setText(VarBox->MajorDir);
	}
}

void Dialog::sLdTaskAlphCurNum(int value)
{
    DWORD bAlphaB = (DWORD)value << 24;
    ui->label_4->setText(QString::number(value, 10));
    VarBox->dAlphaColor[VarBox->setMax] = bAlphaB + (VarBox->dAlphaColor[VarBox->setMax] & 0xffffff);
    if (!VarBox->tray->beautifyTask->isActive())
        VarBox->tray->beautifyTask->start();
}

void Dialog::sLdIconAlphCurNum(int value)
{
    ui->label_6->setText(QString::number(value, 10));
    VarBox->bAlpha[VarBox->setMax] = value;
    if (!VarBox->tray->beautifyTask->isActive())
        VarBox->tray->beautifyTask->start();
}

void Dialog::sLdUpdateTimeCurNum(int value)
{
    ui->label_17->setText(QString::number(value, 10));
    VarBox->tray->beautifyTask->setInterval(value);
    VarBox->RefreshTime = value;
    if (!VarBox->tray->beautifyTask->isActive())
        VarBox->tray->beautifyTask->start();
}

void Dialog::on_chkTimeUnit_min_clicked()
{
    QSettings settings(TASK_DESK_SUB, QSettings::NativeFormat);
    if (settings.contains(SHOW_SECONDS_IN_SYSTEM_CLOCK))
        settings.remove(SHOW_SECONDS_IN_SYSTEM_CLOCK);
    jobTip->showTip("更改成功！");
}

void Dialog::on_chkTimeUnit_sec_clicked()
{
    QSettings settings(TASK_DESK_SUB, QSettings::NativeFormat);
    if (!settings.contains(SHOW_SECONDS_IN_SYSTEM_CLOCK))
    {
        settings.setValue(SHOW_SECONDS_IN_SYSTEM_CLOCK, 1);
    }
    jobTip->showTip("更改成功！");
}

void Dialog::on_radioButton_3_clicked()
{
    VarBox->aMode[VarBox->setMax] = ACCENT_STATE::ACCENT_DISABLED;
}

void Dialog::on_radioButton_4_clicked()
{
    if ((VarBox->aMode[0] == ACCENT_STATE::ACCENT_DISABLED)&&(VarBox->aMode[1] == ACCENT_STATE::ACCENT_DISABLED))
    {
        VarBox->tray->beautifyTask->start(VarBox->RefreshTime);
    }
    VarBox->aMode[VarBox->setMax] = ACCENT_STATE::ACCENT_ENABLE_TRANSPARENTGRADIENT;
}


void Dialog::on_radioButton_5_clicked()
{
    if ((VarBox->aMode[0] == ACCENT_STATE::ACCENT_DISABLED)&&(VarBox->aMode[1] == ACCENT_STATE::ACCENT_DISABLED))
    {
        VarBox->tray->beautifyTask->start(VarBox->RefreshTime);
    }
    VarBox->aMode[VarBox->setMax] = ACCENT_STATE::ACCENT_ENABLE_BLURBEHIND;
}

void Dialog::on_radioButton_6_clicked()
{
    if ((VarBox->aMode[0] == ACCENT_STATE::ACCENT_DISABLED)&&(VarBox->aMode[1] == ACCENT_STATE::ACCENT_DISABLED))
    {
        VarBox->tray->beautifyTask->start(VarBox->RefreshTime);
    }
    VarBox->aMode[VarBox->setMax] = ACCENT_STATE::ACCENT_ENABLE_ACRYLICBLURBEHIND;
}

void Dialog::on_radioButton_11_clicked()
{
    QSettings settings(TASK_DESK_SUB, QSettings::NativeFormat);
    settings.setValue(TASKBAR_ACRYLIC_OPACITY, 10);
    jobTip->showTip("更改成功！");
}

void Dialog::on_radioButton_10_clicked()
{
    QSettings settings(TASK_DESK_SUB, QSettings::NativeFormat);
    settings.setValue(TASKBAR_ACRYLIC_OPACITY, 5);
    jobTip->showTip("更改成功！");
}

void Dialog::on_radioButton_12_clicked()
{
    QSettings settings(TASK_DESK_SUB, QSettings::NativeFormat);
    settings.setValue(TASKBAR_ACRYLIC_OPACITY, 0);
    jobTip->showTip("更改成功！");
}

void Dialog::on_radioButton_7_clicked()
{
    VarBox->iPos = TaskBarCenterState::TASK_LEFT;
}

void Dialog::on_radioButton_8_clicked()
{
    if (VarBox->iPos == TaskBarCenterState::TASK_LEFT)
        VarBox->tray->centerTask->start();
    qout << "居中点击";
    VarBox->iPos = TaskBarCenterState::TASK_CENTER;
}

void Dialog::on_radioButton_9_clicked()
{
    if (VarBox->iPos == TaskBarCenterState::TASK_LEFT)
        VarBox->tray->centerTask->start();
    VarBox->iPos = TaskBarCenterState::TASK_RIGHT;
}

void Dialog::changeType(BOOL ok)
{
    VarBox->setMax = ok;
    ui->sLdTaskAlph->setValue(VarBox->dAlphaColor[ok] >> 24);
    ui->sLdIconAlph->setValue(VarBox->bAlpha[ok]);
	ui->pushButton_3->setStyleSheet(getStyleSheet());
    switch (VarBox->aMode[ok])
	{
    case (ACCENT_STATE::ACCENT_DISABLED):
		ui->radioButton_3->setChecked(true);
		break;
    case (ACCENT_STATE::ACCENT_ENABLE_TRANSPARENTGRADIENT):
		ui->radioButton_4->setChecked(true);
		break;
    case (ACCENT_STATE::ACCENT_ENABLE_BLURBEHIND):
		ui->radioButton_5->setChecked(true);
		break;
    case (ACCENT_STATE::ACCENT_ENABLE_ACRYLICBLURBEHIND):
		ui->radioButton_6->setChecked(true);
		break;
	default:
		break;
	}
}

void Dialog::on_pushButton_3_clicked()
{
    uint32_t col = VarBox->dAlphaColor[VarBox->setMax] & 0xffffff;
    QColor color = QColorDialog::getColor(QColor(col & 0xff, (col >> 8) & 0xff, col >> 16), ui->frame, "颜色");
	if (color.isValid())
	{
		short r = color.red(), g = color.green(), b = color.blue();
        QString str = QString("QPushButton{background-color: rgb(%1, %2, %3);border-radius: 3px; border: 1px solid black;}").arg(QString::number(r), QString::number(g), QString::number(b));
		ui->pushButton_3->setStyleSheet(str);
        VarBox->dAlphaColor[VarBox->setMax] = (ui->sLdTaskAlph->value() << 24) + RGB(r, g, b);
	}
}

void Dialog::cBxstandardNameCurIndex(int index)
{
    ui->lineNewName->setText(VarBox->CustomNames[index]);
}


void Dialog::setCustomName()
{
	QString str = ui->lineNewName->text();
    short i = ui->cBxstandardName->currentIndex();
    if (VarBox->CustomNames[i].compare(str))
	{
		if (str.isEmpty())
        {
			QMessageBox::warning(this, "警告", "名称不能为空！", QMessageBox::Ok, QMessageBox::Ok);
            ui->lineNewName->setText(VarBox->CustomNames[i]);
        }
		else
		{
            const wchar_t patrn[] = L"[`~!@#$%^&-+=\\?:\"|,/;'\\[\\]·~！@#￥%……&*（）+=\\{\\}\\|《》？：“”【】、；‘'，。\\、\\-]";
            for (wchar_t c: str.toStdWString())
            {
                if (wcschr(patrn, c))
                {
                    QMessageBox::warning(this, "警告", "名称含有非法字符！", QMessageBox::Ok, QMessageBox::Ok);
                    return;
                }
            }
            QString new_name = (i == 7) ? VarBox->MajorDir + "\\" + str : VarBox->get_wal_path() + "\\" + str;
            QDir dir;
            if (dir.exists(new_name))
            {
                if (QMessageBox::question(this, "冲突", "目标路径下有一个同名文件夹，您想直接更换文件夹而不迁移图片吗？",
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                {
                    VarBox->sigleSave("Dirs", VarBox->StandardNames[i][0], VarBox->CustomNames[i] = str);
                    jobTip->showTip("更改名称成功！");
                }
                else
                {
                    jobTip->showTip("放心，我什么也没做！");
                    ui->lineNewName->setText(VarBox->CustomNames[i]);
                }
            }
            else
            {
                QString old_name;
                if (i == 7)
                    old_name = VarBox->MajorDir + "\\" + VarBox->CustomNames[7];
                else
                    old_name = VarBox->get_wal_path() + "\\" + VarBox->CustomNames[i];
                if (dir.exists(old_name))
                    dir.rename(old_name, new_name);
                else
                    dir.mkdir(new_name);
                VarBox->CustomNames[i] = str;
                VarBox->sigleSave("Dirs", VarBox->StandardNames[i][0], str);
                jobTip->showTip("更换名称成功！");
            }
		}
	}
    else
    {
        jobTip->showTip("似乎什么也没做。");
    }
}

void Dialog::on_radioButton_13_clicked()
{
    ui->lineEdit->setText(qApp->applicationDirPath().replace("/", "\\"));
    ui->pushButton_4->setText("确定");
}


void Dialog::on_radioButton_14_clicked()
{
    ui->lineEdit->setText("点击右侧按钮选择文件");
    ui->pushButton_4->setText("选择");
}

void Dialog::on_pushButton_4_clicked()
{
    if (ui->pushButton_4->text().compare("确定"))
	{
		QString titile = "请选择一个文件夹";
        QString dir = QFileDialog::getExistingDirectory(ui->frame, titile, QStandardPaths::writableLocation(QStandardPaths::HomeLocation), QFileDialog::ShowDirsOnly);
        ui->pushButton_4->setText("确定");
        if (!dir.isEmpty())
		{
            ui->lineEdit->setText(dir.replace("/", "\\"));
		}
        return;
	}
    VarBox->PathToOpen = ui->lineEdit->text();
    VarBox->get_son_dir(VarBox->PathToOpen);
    VarBox->sigleSave("Dirs", "OpenDir", VarBox->PathToOpen);
    jobTip->showTip("更换路径成功！");
}

void Dialog::on_pBtn_Save_Tran_Info_clicked()
{
    QString str_1 = ui->line_APP_ID->text().trimmed();
    QString str_2 = ui->line_PASS_WORD->text().trimmed();
    if (str_1.isEmpty() || str_2.isEmpty())
        QMessageBox::information(ui->frame, "提示", "APP ID或密钥不能为空！");
    else
    {
        QFile file(VarBox->get_dat_path() + "\\AppId.txt");
        if (file.open(QIODevice::WriteOnly))
        {
            if (VarBox->AppId) delete [] VarBox->AppId;
            if (VarBox->PassWord) delete [] VarBox->PassWord;
            VarBox->HaveAppRight = true;
            qout << 1;
            unsigned char c[3] = {0xEF, 0xBB, 0xBF}; size_t len = 0;
            file.write((char*)c, 3 * sizeof (char));
            file.write("APP ID\t", 7 * sizeof (char));
            QByteArray s1 = str_1.toUtf8(); len = s1.length();
            qout << 2;
            VarBox->AppId = new char[len+1]; memcpy_s(VarBox->AppId, len, s1, len);
            VarBox->AppId[len] = 0;
            file.write(VarBox->AppId, len*sizeof (char));
            file.write("\nPASSWORD\t", 10 * sizeof(char));
            QByteArray s2 = str_2.toUtf8(); len = s2.length();
            qout << 3;
            VarBox->PassWord = new char[len+1]; memcpy_s(VarBox->PassWord, len, s2, len);
            VarBox->PassWord[len] = 0;
            file.write(VarBox->PassWord, len*sizeof (char));
            file.write("\n", sizeof (char));
            file.close();
            jobTip->showTip("记录成功！");
            *const_cast<bool*>(VarBox->FirstUse) = true;
        }
        else
        {
            qout << VarBox->get_dat_path();
            QMessageBox::warning(ui->frame, "错误", "文件写入失败。");
        }
    }
}

inline void del_file(Dialog *di ,QString str)
{
    if (QFile::exists(str))
    {
        if (QMessageBox::question(di, "提示", "本操作不可逆，是否继续？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)==QMessageBox::Yes)
        {
            if (QFile::remove(str))
            {
                di->jobTip->showTip("删除成功!");
            }
            else
                QMessageBox::critical(di, "出错", "无法删除，可能是权限不够！");
        }
    }
    else
    {
        di->jobTip->showTip("文件本就不存在！");
    }
}

void Dialog::on_pushButton_7_clicked()
{
    QString str = VarBox->get_ini_path();
    if (QFile::exists(str))
    {
        if (QMessageBox::question(this, "提示", "将要清除所有设置并退出软件，是否继续？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)==QMessageBox::Yes)
        {
            if (QFile::remove(str))
            {
                jobTip->showTip("删除成功!");
                setEnabled(false);
                VarBox->form->setEnabled(false);
                VarBox->RunApp = false;
                if (QMessageBox::question(this, "提示", "您想退出后重启软件吗？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)==QMessageBox::Yes)
                    qApp->exit(RETCODE_RESTART);
                else
                    qApp->quit();
            }
            else
                QMessageBox::critical(this, "出错", "无法删除，可能是权限不够！");
        }
    }
    else
    {
        jobTip->showTip("文件本就不存在！");
    }
}

void Dialog::on_pushButton_2_clicked()
{
    close();
}


void Dialog::on_pushButton_5_clicked()
{
    VarBox->CurTheme = static_cast<COLOR_THEME>(ui->comboBox_3->currentIndex());
    QSettings IniWrite(VarBox->get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup("UI");
    IniWrite.setValue("ColorTheme", (int)VarBox->CurTheme);
    ui->frame->setStyleSheet(QString("QFrame{background-color:rgba(%1);}QLabel{border-radius: 3px;background-color: transparent;}Line{background-color:black};").arg(color_theme[static_cast<int>(VarBox->CurTheme)]));
    jobTip->showTip("设置成功！");
    qDebug() << (int)VarBox->CurTheme;
}



void Dialog::on_pushButton_11_clicked()
{
    QString str = ui->linePictuerPath->text();
    QDir dir;
    if (dir.exists(str) || (!str.isEmpty() && dir.mkdir(str)))
        VarBox->MajorDir = str.replace("/", "\\");
    else
    {
        if (dir.exists(VarBox->MajorDir) || dir.mkdir(VarBox->MajorDir))
            ui->linePictuerPath->setText(VarBox->MajorDir.replace("/", "\\"));
        else
            VarBox->MajorDir = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    }
    VarBox->get_wal_path();
    jobTip->showTip("修改成功！");
}

// 展示版本信息
void Dialog::on_pushButton_13_clicked()
{
    jobTip->showTip(QString("版本号：") + VarBox->Version, 1000);
}

// 检查是否有更新 https://gitee.com/yjmthu/Speed-Box/raw/main/Update.json
void Dialog::on_pushButton_10_clicked()
{
    static bool wait = false;
    if (wait)
    {
        QMessageBox::warning(this, "提示", "已经在检查更新中，可能是网速比较慢，请勿重复点击！");
        return;
    }
    else
        wait = true;

    std::thread thrd; QEventLoop loop;

    connect(this, &Dialog::finished, &loop, std::bind([this](QEventLoop* loop, std::thread* thrd, bool success, const char* str){
        if (thrd->joinable()) thrd->join(); loop->quit();
        if (!VarBox->RunApp)     // 用户在检查过程中退出了软件
        {
            if (success) delete [] str;
            return;
        }
        if (!success)
        {
            jobTip->showTip(str);
            return ;
        }
        if (!strcmp(VarBox->Version, str))
        {
            jobTip->showTip("当前已经是最新版本, 不过你仍可尝试下载更新！", 1000);
            ui->pushButton_12->setEnabled(true);
        }
        else
        {
            if (VARBOX::versionBefore(VarBox->Version, str))
            {
                jobTip->showTip(((QString("\t有新版本已经出现！\n当前版本：")+=VarBox->Version)+="; 最新版本：")+=str, 2000);
                ui->pushButton_12->setEnabled(true);
            }
            else
            {
                jobTip->showTip(str, 800);
                jobTip->showTip("更新失败，当前版本过高，请手动打开官网更新。", 3000);
            }
        }
        delete [] str;
    }, &loop, &thrd, std::placeholders::_1, std::placeholders::_2));

    thrd = std::thread([this](){
        std::string str;
        if (!VARBOX::getWebCode("https://gitee.com/yjmthu/Speed-Box/raw/main/update/update.json", str, false) || str.empty())
        {
            emit finished(false, "下载失败！");
            return;
        }
        const YJson json(str);
        if (json.getType() != YJSON_TYPE::YJSON_OBJECT)
        {
            emit finished(false, "Gitee源出现问题！");
            return;
        }
        emit finished(true, StrJoin<char>(json.find("Latest Version")->getValueString()));
    });
    loop.exec();
    wait = false;
}

// 下载更新  https://gitee.com/yjmthu/Speed-Box/raw/main/Update.json
void Dialog::on_pushButton_12_clicked()
{
    static bool wait = false;
    if (wait)
    {
        QMessageBox::warning(this, "提示", "已经在下载更新中，请勿重复点击！");
        return;
    }
    else
        wait = true;
    if (VarBox->isOnline(false))
    {
        QMessageBox::information(this, "提示", "点击确认开始下载，下载时间一般不会很长，请耐心等待。");
    }
    else
    {
        QMessageBox::information(this, "提示", "没有网络！");
        return;
    }
    std::thread thrd; QEventLoop loop;

    connect(this, &Dialog::finished, &loop, std::bind([this](QEventLoop* loop, std::thread* thrd, bool success, const char* str){
        if (thrd->joinable()) thrd->join(); loop->quit();
        if (!success)
        {
            jobTip->showTip(str, 800);
            return ;
        }
        if (!VarBox->RunApp) return;        // 用户在检查过程中退出了软件
        if (QMessageBox::information(this, "提示", "更新已经下载完成，点击确认重启软件后完成更新！", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
        {
            VarBox->RunApp = false;
            qApp->exit(RETCODE_UPDATE);
        }
        else
            jobTip->showTip("成功取消更新。", 700);
    }, &loop, &thrd, std::placeholders::_1, std::placeholders::_2));

    thrd = std::thread([this](){
        std::string str; //jobTip->showTip("请耐心等待几秒...");
        if (!VARBOX::getWebCode("https://gitee.com/yjmthu/Speed-Box/raw/main/update/update.json", str, false) || str.empty())
        {
            emit finished(false, "下载失败！");
            return;
        }
        YJson json(str);
        if (json.getType() != YJSON_TYPE::YJSON_OBJECT)
        {
            emit finished(false, "Gitee源出现问题！");
            return;
        }
        YJson *qtVersion = json.find("Qt Version");
        if (qtVersion->getType() != YJSON_TYPE::YJSON_STRING || strcmp(qtVersion->getValueString(), VarBox->Qt))
        {
            jobTip->showTip("更新失败，当前Qt版本过低，请手动打开官网下载更新！", 3000);
            return;
        }
        YJson *urls = json.find("Files"); YJson *child = nullptr;
        if (urls && urls->getType() == YJSON_TYPE::YJSON_OBJECT)
        {
            if (!(child = urls->find("main")))
            {
                emit finished(false, "下载失败！");
                return;
            }
            const char* url1 = child->getValueString();
            if (!(child = urls->find("update")))
            {
                emit finished(false, "下载失败！");
                return;
            }
            const char* url2 = child->getValueString();
            qout << "文件地址：" << url1 << url2;
            QString temp_str_1 = VarBox->get_dat_path()+"\\SpeedBox.exe";
            QString temp_str_2 = qApp->applicationDirPath().replace("/", "\\")+"\\update.exe";
            if (QFile::exists(temp_str_1)) QFile::remove(temp_str_1);
            if (QFile::exists(temp_str_2)) QFile::remove(temp_str_2);

            if (VARBOX::downloadImage(url1, temp_str_1, false)
                    &&
                VARBOX::downloadImage(url2, temp_str_2, false))
            {
                emit finished(true);
            }
            else
                emit finished(false, "下载失败！");
        }
        else
        {
            emit finished(false, "未知原因，下载失败！");
        }
    });
    loop.exec();
    wait = false;
}


void Dialog::on_pushButton_14_clicked()
{
    ShellExecuteA(NULL, "open", "https://yjmthu.github.io/Speed-Box", NULL, NULL, SW_SHOW);
}


void Dialog::on_toolButton_2_clicked()
{
    QString titile = "请选择一个文件夹";
    QString dir = QFileDialog::getExistingDirectory(NULL, titile, VarBox->MajorDir, QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty())
    {
        ui->linePictuerPath->setText(QDir::toNativeSeparators(dir));
    }
}


void Dialog::on_pushButton_6_clicked()
{
    FormSetting d;
    d.move(frameGeometry().x()+(width()-d.width())/2, frameGeometry().y()+(height()-d.height())/2);
    d.exec();
}


void Dialog::on_checkBox_4_clicked(bool checked)
{
    QSettings IniWrite(VarBox->get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup("UI");
    IniWrite.setValue("ControlDesktopIcon", checked);
    if (checked)
    {
        VarBox->ControlDesktopIcon = new DesktopMask;
    }
    else
    {
        delete VarBox->ControlDesktopIcon;
        VarBox->ControlDesktopIcon = nullptr;
    }
    jobTip->showTip("设置并保存成功！");
}

