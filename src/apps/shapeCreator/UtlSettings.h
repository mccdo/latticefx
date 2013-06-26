#ifndef UTLSETTINGS_H
#define UTLSETTINGS_H

#include <QSettings>
#include <QComboBox>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QListWidget>

class UtlSettings
{
public:
    static void initComboBox( QSettings *pset, QComboBox *pCtl, const QString &name, const QVariant &def );
    static void initPlainText( QSettings *pset, QPlainTextEdit *pCtl, const QString &name, const QString &def );
    static void initCheckBox( QSettings *pset, QCheckBox *pCtl, const QString &name, bool def );
    static void initRadioBtn( QSettings *pset, QRadioButton *pCtl, const QString &name, bool def );

    static void saveComboBox( QSettings *pset, QComboBox *pCtl, const QString &name );
    static void savePlainText( QSettings *pset, QPlainTextEdit *pCtl, const QString &name );
    static void saveCheckBox( QSettings *pset, QCheckBox *pCtl, const QString &name );
    static void saveRadioBtn( QSettings *pset, QRadioButton *pCtl, const QString &name );

	static int getSelectedValueInt( QComboBox *pCtl);
	static int getCheckedItems( QListWidget *pCtl, std::vector<int> *pIndexs );
};

#endif // UTLSETTINGS_H
