#include "widget.h"

#include "./ui_widget.h"
#include "json.h"

#include <QDebug>
#include <QGuiApplication>
#include <QMessageBox>
#include <QSize>
#include <fstream>
#include <qthread.h>
#include <stdexcept>


Widget::Widget(QWidget* parent) : QWidget(parent), ui(new Ui::InitWidget) {
	this->ui->setupUi(this);
	this->last_pos = QPoint(0, 0);


	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow);
	this->setAttribute(Qt::WA_NoSystemBackground);
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->setAttribute(Qt::WA_TransparentForMouseEvents);

	this->load_config();
	this->effect = new QGraphicsDropShadowEffect(this);
	this->effect->setBlurRadius(20);
	this->effect->setOffset(0, 0);
	this->effect->setColor(QColor(0, 0, 0, 100));
	this->ui->label->setGraphicsEffect(this->effect);
	this->ui->label->setFont(QFont("Microsoft JhengHei UI", 28, QFont::Black));
	this->ui->label->setStyleSheet(QString("QLabel { color : ") + this->text_color + QString("; }"));
	this->ui->label->setText("Loading...");


	QSize screenrect = QGuiApplication::primaryScreen()->size();
	this->setGeometry(0, 20, screenrect.width() - 10, height() + 20);
	this->ui->label->setGeometry(10, 20, width() - 20, height() - 40);

	connect(this, SIGNAL(err_msg_box_setup_signal()), this, SLOT(load_err_msg_box()));
}

Widget::~Widget() { delete ui; }

void Widget::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		this->last_pos = event->globalPosition().toPoint();
	} else if (event->button() == Qt::RightButton) {
		if (QMessageBox::question(this, "Exit", "Do you want to close the application?",
								  QMessageBox::Yes | QMessageBox::No)
			== QMessageBox::Yes) {
			qApp->exit(0);
		}
	}
}


void Widget::mouseMoveEvent(QMouseEvent* event) {
	QPoint new_pos = event->globalPosition().toPoint() - last_pos;
	this->move(x() + new_pos.x(), y() + new_pos.y());
	this->last_pos = event->globalPosition().toPoint();
}


void Widget::load_config(void) {
	this->text_color = "cyan";
	JSON config = JSON(QFile("./config/config.json"));
	if (config.get_error().error == QJsonParseError::NoError) {
		auto raw_text_color = config.get("text_color");
		qDebug() << "config file loaded: " << raw_text_color;
		if (raw_text_color.isString()) {
			auto _text_color = raw_text_color.toString();
			if (!_text_color.isEmpty()) {
				this->text_color = _text_color;
			}
		}
		qDebug() << "text color: " << this->text_color;
	} else {
		qDebug() << "config file not loaded, using default values";
	}
}

void Widget::load_err_msg_box(void) {
	qDebug() << "loading error message box";
	if (!this->msg_box_title.isEmpty() && !this->msg_box_text.isEmpty()) {
		qDebug() << "loading error message box";
		if (QMessageBox::critical(this, this->msg_box_title, this->msg_box_text, QMessageBox::Ok) == QMessageBox::Ok) {
			qDebug() << "exiting...";
			qApp->exit(1);
		}
	}
}

void Widget::SetLyricsLabel(const QString text) { this->ui->label->setText(text); }


void Widget::SetErrMsgBox(QString title, QString text) {
	if (!this->msg_box_title.isEmpty() && !this->msg_box_text.isEmpty()) {
		qDebug() << "error message box already set";
		return;
	}
	qDebug() << title << ":" << text;
	std::ofstream log("crash.log");
	if (log.is_open()) {
		log << text.toStdString() << std::endl;
		log.close();
		qDebug() << "log file written";
	}
	this->msg_box_title = title;
	this->msg_box_text = text;
	emit err_msg_box_setup_signal();
	QThread::sleep(10);
}