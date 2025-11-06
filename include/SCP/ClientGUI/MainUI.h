#ifndef CLIENTGUI_MAINUI_H_
#define CLIENTGUI_MAINUI_H_

#include <string_view>
#include <optional>

#include <wx/wx.h>
#include <wx/sizer.h>

#include "SCP/Client/ChatClient.h"
#include "SCP/ClientGUI/ConnectDialog.h"
#include "SCP/ClientGUI/ConnectInfo.h"

#define SCP_MAINUI_CONNECT 1
#define SCP_MAINUI_DISCONNECT 2
#define SCP_MAINUI_INPUTBOX 3
#define SCP_MAINUI_SENDBTN 4

#define SCP_MAINUI_EVT_CONNECT 100
#define SCP_MAINUI_EVT_CHATMSG 101
#define SCP_MAINUI_EVT_DISCONNECT 102

wxDECLARE_EVENT(MY_NEW_TYPE, wxCommandEvent);

namespace SCP::ClientGUI
{
    class MainUI : public wxFrame, SCP::Client::ChatClient
    {
    private:
        wxTextCtrl* m_ChatBox;
        wxTextCtrl* m_InputBox;
        wxButton* m_SendButton;

        void SendChatMessage(const std::string_view&);

        //bool m_Connecting;
    public:
        MainUI();

        void OnConnectButton(wxCommandEvent&);
        void OnDisconnectButton(wxCommandEvent&);

        void OnTextInput(wxCommandEvent&);
        void OnSendMessage(wxCommandEvent&);

        void OnClientConnect(wxCommandEvent&);
        void OnClientMsg(wxCommandEvent&);
        void OnClientDisconnect(wxCommandEvent&);

        void OnConnect(std::optional<std::string>) override;
        void OnMessage(std::string) override;
        void OnDisconnect(std::optional<std::string>) override;
    };
}

#endif