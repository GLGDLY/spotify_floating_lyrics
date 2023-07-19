#ifndef WIDGET_DEF
#define WIDGET_DEF

#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QPoint>
#include <QWidget>


namespace Ui {
	class InitWidget;
}

class Widget : public QWidget {
	Q_OBJECT

public:
	Widget(QWidget* parent = nullptr);
	~Widget();

	void SetLyricsLabel(const QString text);
	void SetLyricsSubLabel(const QString text);
	void SetErrMsgBox(QString title, QString text);

private:
	Ui::InitWidget* ui;
	QPoint last_pos;
	QString text_color = "cyan";
	QString msg_box_title, msg_box_text;
	QGraphicsDropShadowEffect* effect;

	void load_config(void);

public slots:
	void load_err_msg_box(void);

signals:
	void err_msg_box_setup_signal(void);

protected:
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
};
#endif
