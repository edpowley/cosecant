#include "stdafx.h"
#include "common.h"
#include "dlg_about.h"
#include "version.h"

Dlg_About::Dlg_About(QWidget *parent, Qt::WFlags flags)
: QDialog(parent, flags)
{
	ui.setupUi(this);

	QString html = ui.textBrowser->toHtml();
	html.replace("COSECANT_VERSION", getVersionString());
	html.replace("COSECANT_BUILD_DATE", versionBuildDate);
	ui.textBrowser->setHtml(html);
}

Dlg_About::~Dlg_About()
{
}
