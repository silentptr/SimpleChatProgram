#ifndef SCP_CLIENTGUI_CONNECTDIALOG_H_
#define SCP_CLIENTGUI_CONNECTDIALOG_H_

#include <wx/wx.h>
#include <wx/dialog.h>

#define SCP_CONNECTDIALOG_CANCEL 5
#define SCP_CONNECTDIALOG_CONNECT 6

namespace SCP::ClientGUI
{
    class ConnectDialog : public wxDialog
    {
    private:
        wxTextCtrl* m_IP, * m_Port, * m_Username;
        wxButton* m_Cancel, * m_Connect;
    public:
        ConnectDialog(wxWindow*);

        void OnCancel(wxCommandEvent&);
        void OnConnect(wxCommandEvent&);
    };
}

#endif