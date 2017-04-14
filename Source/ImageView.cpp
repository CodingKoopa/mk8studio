#include <QPainter>

#include "ImageView.h"

ImageView::ImageView(QImage* image, QWidget* parent) : QWidget(parent), image(image)
{
}

void ImageView::paintEvent(QPaintEvent* event)
{
  QPainter canvas(this);
  canvas.drawImage(0, 0, *image);

  event = event;

  update();
}
