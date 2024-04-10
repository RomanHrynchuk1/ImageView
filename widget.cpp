#include "widget.h"
#include "ui_widget.h"

#include <QDateTime>

using std::sscanf;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , watcher(new QFileSystemWatcher(this))
{
    ui->setupUi(this);

    this->setWindowFlag(Qt::FramelessWindowHint);

    current_count = 0;

    connect(watcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(directoryModifying()));
    QTimer::singleShot(200, this, SLOT(init()));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::init() {
    directoryChanged();
}

void Widget::readData() {
//    qDebug() << CONFIG_NAME << FONT_SIZE << HEADING_HEIGHT;
    QFile file(CONFIG_NAME);
    if (!file.exists()) error(1);
    if (!file.open(QFile::ReadOnly | QFile::Text)) error(2);

    QTextStream in(&file);
    fileName = readLine(in);
    QString img_size = readLine(in);
    QString window_pos = readLine(in);
    QString red_rect = readLine(in);
    QString blue_rect = readLine(in);

    file.close();

    fileName.replace("\"", "");

    bool flag = true;

    flag &= sscanf(img_size.toStdString().c_str(), "%d %d", &img_wid, &img_hei) == 2;
    flag &= sscanf(window_pos.toStdString().c_str(), "%d %d", &window_x, &window_y) == 2;

    flag &= sscanf(red_rect.toStdString().c_str(), "%d %d %d %d", &r1x, &r1y, &r2x, &r2y) == 4;
    flag &= sscanf(blue_rect.toStdString().c_str(), "%d %d %d %d", &b1x, &b1y, &b2x, &b2y) == 4;

    if (!flag) {
        error(3);
    }

    int hei = 3 * (HEADING_HEIGHT + img_hei);
    this->setGeometry(window_x, window_y, img_wid, hei);
    this->show();

    QFileInfo fileInfo(fileName);
    qDebug() << fileInfo.absolutePath();
    watcher->addPath(fileInfo.absolutePath());
}

QString Widget::readLine(QTextStream &in) {
    if (in.atEnd()) error(3);
    QString str = in.readLine();
    return str;
}

void Widget::error(int id) {
    QString error_text = "";
    switch (id) {
    case 1:
        error_text = "Can't find the \"config.txt\" file.";
        break;
    case 2:
        error_text = "Can't open the \"config.txt\" file.";
        break;
    case 3:
        error_text = QString("Config file content is wrong.\n"
                             "\nExample:\n"
                             "  IMG_PATH.JPG     # image path (absolute or relative)\n"
                             "  220 250          # size of one image\n"
                             "  50 50            # window top-left position\n"
                             "  100 100 150 150  # red rectangle\n"
                             "  160 100 200 200  # blue rectangle\n"
                             "\nPlease check it and restart the application.");
        break;
    default:
        error_text = "Error occured.";
    }

    QMessageBox::warning(this, "Warning", error_text, "OK");
    exit(0);
}

void Widget::paintEvent(QPaintEvent *ev) {
    Q_UNUSED(ev);
    QPainter painter(this);

    // Calculate spacing and dimensions
    int imageWidth = img_wid; // Assuming all images have same width
    int imageHeight = img_hei;
    int headingHeight = HEADING_HEIGHT;
    int padding_left = 10;
    int spacing = 0; // Adjust spacing as needed
    int totalHeight = 0;

    // Draw each image with heading
    for (int i = 0; i < current_count; ++i) {
        int yPosition = totalHeight;

        // Draw heading rectangle
        painter.fillRect(0, yPosition, imageWidth, headingHeight, Qt::lightGray);

        // Draw heading text
        QString text = QString::number(i+1);
        if (i == 0)
            text += " - Newest";
        else if (i + 1 == current_count)
            text += " - Latest";
        painter.setFont(QFont("Arial", 9)); // Adjust font as needed
        painter.setPen(Qt::black);
        painter.drawText(QRect(padding_left, yPosition, imageWidth - 2 * spacing, headingHeight), Qt::AlignLeft | Qt::AlignVCenter, text);

        // Draw image below the heading
        yPosition += headingHeight + spacing;
        painter.drawImage(0, yPosition, images[i]);

        // Set pen color to red
        QPen pen(Qt::red);
        pen.setWidth(RECT_WIDTH);
        painter.setPen(pen);

        // Draw a red rectangle
        QRect rect(r1x, yPosition + r1y, r2x - r1x, r2y - r1y); // Example rectangle coordinates (x, y, width, height)
        painter.drawRect(rect);

        // Set pen color to green
        QPen pen2(Qt::green);
        pen2.setWidth(RECT_WIDTH);
        painter.setPen(pen2);

        // Draw a red rectangle
        QRect rect2(b1x, yPosition + b1y, b2x - b1x, b2y - b1y); // Example rectangle coordinates (x, y, width, height)
        painter.drawRect(rect2);

        totalHeight = yPosition + imageHeight;
    }
}

void Widget::directoryChanged() {
    qDebug() << "directoryChanged()";
    readData();
    if (!QFile::exists(fileName) || !QImage().load(fileName))
        return;
    QImage image = QImage(fileName).scaled(img_wid, img_hei);
    if (image.isNull())
        return;
    static qint64 lastModifyTime = -1000;
    qDebug() << "[fileChanged]  fileName :" << fileName;
    if (abs(lastModifyTime - QDateTime::currentDateTime().currentMSecsSinceEpoch()) < 50) {
        qDebug() << "    canceled because of too small interval. (less than 0.05s)";
        return;
    }
    for (int i = current_count; i > 0; i --) {
        images[i] = images[i-1].copy();
    }
    images[0] = image.copy();
    current_count = qMin(3, current_count + 1);

    lastModifyTime = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    update();
}

void Widget::directoryModifying() {
    QTimer::singleShot(300, this, SLOT(directoryChanged()));
}
