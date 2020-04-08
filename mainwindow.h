#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QNetworkAccessManager>
#include "QtWebSockets/QWebSocket"
#include "QtWebSockets/QWebSocketServer"
#include "QListWidgetItem"

#include "QJsonDocument"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_boutonRecherche_clicked();
    void pixabay_reply(QNetworkReply * reply);
    void wss_client_in();
    void wss_client_kicked_out();
    void wss_client_out();
    void wss_client_sent_text_message(QString message);
    void wss_client_sent_binary_data(QByteArray data);
    void settings_save();
    void settings_load();
    // Test
    void test_item_click(QListWidgetItem *);
    void pixabay_test_results_from(QString file);
    void pixabay_display_results(QJsonDocument json);
    void image_download_finished(QNetworkReply *);


private:
    struct Nem {
        QNetworkAccessManager * nm;
        void * user_data;
    };
    void nems_prepare(
        QList<Nem> &nems,
        QString const &url,
        void * __restrict const user_data);
    Ui::MainWindow *ui;
    QNetworkAccessManager nm;
    QList<Nem> images_nems;
    QWebSocketServer * wss;
    QList<QWebSocket *> clients_list;
    QJsonDocument pixabay_response;
};
#endif // MAINWINDOW_H
