#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QWidget>

class ImageView : public QWidget
{
    Q_OBJECT
    public:
    explicit ImageView(QImage* image, QWidget *parent = 0);

    protected:
    void paintEvent(QPaintEvent *event);

    private:
    QImage* image;

    signals:

    public slots:
};

#endif // IMAGEVIEW_H
