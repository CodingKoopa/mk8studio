#pragma once

#include <QWidget>

class ImageView : public QWidget
{
  Q_OBJECT
public:
  explicit ImageView(QImage* image, QWidget* parent = 0);

protected:
  void paintEvent(QPaintEvent* event);

private:
  QImage* image;

signals:

public slots:
};
