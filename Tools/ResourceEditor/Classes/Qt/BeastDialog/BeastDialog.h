#ifndef BEAST_DIALOG
#define BEAST_DIALOG


#include <QWidget>
#include <QScopedPointer>
#include <QPointer>

#include "ui_BeastDialog.h"


class SceneEditor2;
class QEventLoop;


class BeastDialog
    : public QWidget
{
    Q_OBJECT

public:
    BeastDialog(QWidget *parent = 0);
    ~BeastDialog();

    void SetScene(SceneEditor2 *scene);
    bool Exec(QWidget *parent = 0);
    QString GetPath() const;

private slots:
    void OnStart();
    void OnCancel();
    void OnBrowse();
    void OnTextChanged();

private:
    void closeEvent( QCloseEvent * event );
    QString GetDefaultPath() const;
    void SetPath( const QString& path );

    QScopedPointer<Ui::BeastDialog> ui;
    QPointer< QEventLoop > loop;
    SceneEditor2 *scene;
    bool result;
};


#endif // BEAST_DIALOG
