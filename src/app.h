#ifndef APP_DEF
#define APP_DEF
#include "./thirdparty/cpp-httplib/httplib.h"

#include <QThread>


class App : public QThread {
	Q_OBJECT
public:
	App(void);

	void run(void);

	bool get_verify_code(QString& code);

private:
	httplib::Server svr;
	QString verify_code;
	bool verify_code_set = false;
};

#endif