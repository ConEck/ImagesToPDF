#include "item.h"

Item::Item(const QString& name, const QString& path, QListWidget* parent) : QListWidgetItem(name, parent), path_(path)
{
}

QString Item::getPath() const
{
  return path_;
}
