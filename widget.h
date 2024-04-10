#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <QTimer>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QTextStream>
#include <QFileSystemWatcher>
#include <QImage>
#include <QPaintEvent>
#include <QPainter>
#include <QFileInfo>

#define CONFIG_NAME "config.txt"

#define FONT_SIZE 9
#define HEADING_HEIGHT 24
#define RECT_WIDTH 3

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;

    QString fileName;
    int img_wid, img_hei;
    int window_x, window_y;
    int r1x, r1y, r2x, r2y;
    int b1x, b1y, b2x, b2y;

    QFileSystemWatcher *watcher;

    int current_count;
    QImage images[5];

    QString readLine(QTextStream &in);
    void readData();


private slots:
    void init();
    void error(int error_id);
    void directoryChanged();
    void directoryModifying();

protected:
    void paintEvent(QPaintEvent *ev);

};
#endif // WIDGET_H
