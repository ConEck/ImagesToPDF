#ifndef ITEM_H
#define ITEM_H

#include <QListWidgetItem>

class Item : public QListWidgetItem
{
public:
  Item(const QString& name, const QString& path, QListWidget* parent = nullptr);

private:
  QString path_;

public:
  QString getPath() const;
};

#endif  // ITEM_H
