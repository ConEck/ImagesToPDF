#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

private:
  Ui::MainWindow* ui_;

protected:
  void dropEvent(QDropEvent* event);
  void dragEnterEvent(QDragEnterEvent* event);
  void dragMoveEvent(QDragMoveEvent* event);
  void dragLeaveEvent(QDragLeaveEvent* event);
  void closeEvent(QCloseEvent* event);

private slots:
  void onConvertButtonClicked();
  void onImageListContextMenuRequest(const QPoint& pos);
  void eraseImage();
  void addImage();
  void onRadioButtonSelectedPathClicked();
  void onRadioButtonImagePathClicked();
  void onSelectPathButtonClicked();
  void onImageListSelectionChanged();
  void onMoveUpButtonClicked();
  void onMoveDownButtonClicked();
  void onSortFromAToZButtonClicked();
  void onSortFromZToAButtonClicked();
  void onFilenameTextChanged(const QString& text);

private:
  void checkEnablingConvertButton();
  void addImages(const QList<QUrl>& url_list);
  void loadSettings();
  void connectWidgets();
  void disconnectWidgets();
};
#endif  // MAINWINDOW_H
