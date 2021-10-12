#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "item.h"
#include <QProcess>
#include <QMouseEvent>
#include <QFileDialog>
#include <QDrag>
#include <QMimeData>
#include <QDropEvent>
#include <iostream>
#include <QSettings>
#include <QMessageBox>
#include <QTextStream>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui_(new Ui::MainWindow)
{
  ui_->setupUi(this);
  connectWidgets();
  loadSettings();
}

MainWindow::~MainWindow()
{
  disconnectWidgets();
  delete ui_;
}

void MainWindow::onConvertButtonClicked()
{
  if (ui_->image_list->count() > 0)
  {
    QStringList arguments;
    QString output_path;
    if (ui_->rb_use_selected_path->isChecked())
    {
      QString path = ui_->le_path->text();
      if (!QDir(path).exists())
      {
        int result = QMessageBox::warning(this, tr("Fault"), tr("Folder does not exist! Create folder?"),
                                          QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (result == QMessageBox::Yes)
        {
          QDir().mkdir(path);
        }
        else
        {
          return;
        }
      }
      output_path += path;
    }
    else
    {
      QString path = reinterpret_cast<Item*>(ui_->image_list->item(0))->getPath();
      QFileInfo info(path);
      output_path += info.absoluteDir().absolutePath();
    }
    QString file_name = ui_->le_file_name->text();
    if (!file_name.endsWith(".pdf"))
    {
      file_name += ".pdf";
    }
    output_path += QDir::separator() + file_name;
    if (QFile(output_path).exists())
    {
      int result = QMessageBox::warning(this, tr("Warning"), tr("%1 already exists! Override?").arg(file_name),
                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (result == QMessageBox::No)
      {
        return;
      }
    }
    arguments.push_back(output_path);
    if (ui_->cb_pdf_side_size->isChecked())
    {
      arguments.push_back("-useimgsize");
    }
    for (int i = 0; i < ui_->image_list->count(); i++)
    {
      QString path = reinterpret_cast<Item*>(ui_->image_list->item(i))->getPath();
      arguments.push_back(path);
    }
    int result = QProcess::execute("podofoimg2pdf", arguments);
    if (result != 0)
    {
      QMessageBox::critical(this, tr("Fault"), tr("An error occurred while saving!"));
    }
    else
    {
      QMessageBox::information(this, tr("Info"), tr("PDF created!"));
    }
  }
}

void MainWindow::onImageListContextMenuRequest(const QPoint& pos)
{
  QPoint global_pos = ui_->image_list->mapToGlobal(pos);
  QMenu menu;
  menu.addAction(tr("add Image"), this, SLOT(addImage()));
  if (ui_->image_list->selectedItems().size() > 0)
  {
    menu.addAction(tr("delete"), this, SLOT(eraseImage()));
  }
  menu.exec(global_pos);
}

void MainWindow::dropEvent(QDropEvent* event)
{
  const QMimeData* mime_data = event->mimeData();
  if (mime_data->hasUrls())
  {
    QList<QUrl> url_list = mime_data->urls();
    addImages(url_list);
  }
  checkEnablingConvertButton();
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
  event->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
  event->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
{
  event->accept();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  QSettings settings("coneck", "ImagesToPDF");
  settings.beginGroup("MainWindow");
  settings.setValue("last_path", ui_->le_path->text());
  settings.setValue("use_path_from_first_image", ui_->rb_use_img_path->isChecked());
  settings.setValue("use_imagesize_as_page_size", ui_->cb_pdf_side_size->isChecked());
  settings.endGroup();
  event->accept();
}

void MainWindow::checkEnablingConvertButton()
{
  if (ui_->image_list->count() > 0 && !ui_->le_file_name->text().isEmpty() &&
      ((ui_->rb_use_selected_path->isChecked() && !ui_->le_path->text().isEmpty()) ||
       ui_->rb_use_img_path->isChecked()))
  {
    ui_->pb_convert->setEnabled(true);
  }
  else
  {
    ui_->pb_convert->setEnabled(false);
  }
}

void MainWindow::addImages(const QList<QUrl>& url_list)
{
  for (const QUrl& url : url_list)
  {
    bool is_image = true;
    try
    {
      QPixmap image(url.toLocalFile());
      is_image = (image.width() > 0);
    }
    catch (...)
    {
      is_image = false;
    }
    if (is_image)
    {
      ui_->image_list->addItem(new Item(url.fileName(), url.toLocalFile(), ui_->image_list));
    }
    else
    {
      std::cout << "Can not add " << url.fileName().toStdString() << std::endl;
    }
  }
}

void MainWindow::loadSettings()
{
  QSettings settings("coneck", "ImagesToPDF");
  settings.beginGroup("MainWindow");
  ui_->le_path->setText(settings.value("last_path", "").toString());
  bool use_image_path = settings.value("use_path_from_first_image", true).toBool();
  ui_->rb_use_img_path->setChecked(use_image_path);
  ui_->rb_use_selected_path->setChecked(!use_image_path);
  if (!use_image_path)
  {
    onRadioButtonSelectedPathClicked();
  }
  ui_->cb_pdf_side_size->setChecked(settings.value("use_imagesize_as_page_size", true).toBool());
  settings.endGroup();
}

void MainWindow::connectWidgets()
{
  connect(ui_->pb_convert, SIGNAL(clicked()), this, SLOT(onConvertButtonClicked()));
  connect(ui_->image_list, SIGNAL(customContextMenuRequested(QPoint)), this,
          SLOT(onImageListContextMenuRequest(QPoint)));
  connect(ui_->rb_use_selected_path, SIGNAL(clicked()), this, SLOT(onRadioButtonSelectedPathClicked()));
  connect(ui_->rb_use_img_path, SIGNAL(clicked()), this, SLOT(onRadioButtonImagePathClicked()));
  connect(ui_->pb_select_path, SIGNAL(clicked()), this, SLOT(onSelectPathButtonClicked()));
  connect(ui_->image_list, SIGNAL(itemSelectionChanged()), this, SLOT(onImageListSelectionChanged()));
  connect(ui_->pb_up, SIGNAL(clicked()), this, SLOT(onMoveUpButtonClicked()));
  connect(ui_->pb_down, SIGNAL(clicked()), this, SLOT(onMoveDownButtonClicked()));
  connect(ui_->pb_A_Z, SIGNAL(clicked()), this, SLOT(ononSortFromAToZButtonClicked()));
  connect(ui_->pb_Z_A, SIGNAL(clicked()), this, SLOT(ononSortFromZToAButtonClicked()));
  connect(ui_->le_file_name, SIGNAL(textChanged(QString)), this, SLOT(onFilenameTextChanged(QString)));
}

void MainWindow::disconnectWidgets()
{
  disconnect(ui_->pb_convert, SIGNAL(clicked()), this, SLOT(onConvertButtonClicked()));
  disconnect(ui_->image_list, SIGNAL(customContextMenuRequested(const QPoint&)), this,
             SLOT(onImageListContextMenuRequest(const QPoint&)));
  disconnect(ui_->rb_use_selected_path, SIGNAL(clicked()), this, SLOT(onRadioButtonSelectedPathClicked()));
  disconnect(ui_->rb_use_img_path, SIGNAL(clicked()), this, SLOT(onRadioButtonImagePathClicked()));
  disconnect(ui_->pb_select_path, SIGNAL(clicked()), this, SLOT(onSelectPathButtonClicked()));
  disconnect(ui_->image_list, SIGNAL(itemSelectionChanged()), this, SLOT(onImageListSelectionChanged()));
  disconnect(ui_->pb_up, SIGNAL(clicked()), this, SLOT(onMoveUpButtonClicked()));
  disconnect(ui_->pb_down, SIGNAL(clicked()), this, SLOT(onMoveDownButtonClicked()));
  disconnect(ui_->pb_A_Z, SIGNAL(clicked()), this, SLOT(ononSortFromAToZButtonClicked()));
  disconnect(ui_->pb_Z_A, SIGNAL(clicked()), this, SLOT(ononSortFromZToAButtonClicked()));
  disconnect(ui_->le_file_name, SIGNAL(textChanged(QString)), this, SLOT(onFilenameTextChanged(QString)));
}

void MainWindow::eraseImage()
{
  const QList<QListWidgetItem*> items = ui_->image_list->selectedItems();
  for (QListWidgetItem* item : items)
  {
    delete item;
  }
  checkEnablingConvertButton();
}

void MainWindow::addImage()
{
  QList<QUrl> url_list = QFileDialog::getOpenFileUrls(this, tr("open images"));
  addImages(url_list);
  checkEnablingConvertButton();
}

void MainWindow::onRadioButtonSelectedPathClicked()
{
  if (ui_->rb_use_selected_path->isChecked())
  {
    ui_->le_path->setEnabled(true);
    ui_->pb_select_path->setEnabled(true);
  }
}

void MainWindow::onRadioButtonImagePathClicked()
{
  if (ui_->rb_use_img_path->isChecked())
  {
    ui_->le_path->setEnabled(false);
    ui_->pb_select_path->setEnabled(false);
  }
}

void MainWindow::onSelectPathButtonClicked()
{
  QString path = QFileDialog::getExistingDirectory(this, tr("select path"), ui_->le_path->text(),
                                                   QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!path.isEmpty())
  {
    ui_->le_path->setText(path);
    checkEnablingConvertButton();
  }
}

void MainWindow::onImageListSelectionChanged()
{
  if (ui_->image_list->selectedItems().size() > 0)
  {
    ui_->pb_up->setEnabled(true);
    ui_->pb_down->setEnabled(true);
  }
  else
  {
    ui_->pb_up->setEnabled(false);
    ui_->pb_down->setEnabled(false);
  }
}

void MainWindow::onMoveUpButtonClicked()
{
  const QModelIndexList index_list = ui_->image_list->selectionModel()->selectedIndexes();
  std::vector<int> indices;
  for (QModelIndex index : index_list)
  {
    if (index.row() == 0)
    {
      return;
    }
    indices.push_back(index.row());
  }
  std::sort(indices.begin(), indices.end());
  for (int index : indices)
  {
    QListWidgetItem* item = ui_->image_list->takeItem(index);
    ui_->image_list->insertItem(index - 1, item);
    item->setSelected(true);
  }
}

void MainWindow::onMoveDownButtonClicked()
{
  const QModelIndexList index_list = ui_->image_list->selectionModel()->selectedIndexes();
  std::vector<int> indices;
  for (QModelIndex index : index_list)
  {
    if (index.row() == ui_->image_list->count() - 1)
    {
      return;
    }
    indices.push_back(index.row());
  }
  std::sort(indices.begin(), indices.end());
  std::reverse(indices.begin(), indices.end());
  for (int index : indices)
  {
    QListWidgetItem* item = ui_->image_list->takeItem(index);
    ui_->image_list->insertItem(index + 1, item);
    item->setSelected(true);
  }
}

void MainWindow::onSortFromAToZButtonClicked()
{
  ui_->image_list->sortItems(Qt::AscendingOrder);
}

void MainWindow::onSortFromZToAButtonClicked()
{
  ui_->image_list->sortItems(Qt::DescendingOrder);
}

void MainWindow::onFilenameTextChanged(const QString& text)
{
  checkEnablingConvertButton();
}
