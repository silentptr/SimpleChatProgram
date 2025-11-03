#ifndef CLIENTGUI_MAINUI_H_
#define CLIENTGUI_MAINUI_H_

#include <string_view>

#include <wx/wx.h>
#include <wx/sizer.h>

#include "SCP/Client/ChatClient.h"
#include "SCP/ClientGUI/ConnectDialog.h"

#define SCP_MAINUI_CONNECT 1
#define SCP_MAINUI_DISCONNECT 2
#define SCP_MAINUI_INPUTBOX 3
#define SCP_MAINUI_SENDBTN 4

namespace SCP::ClientGUI
{
    class MainUI : public wxFrame, SCP::Client::ChatClient
    {
    private:
        wxTextCtrl* m_ChatBox;
        wxTextCtrl* m_InputBox;
        wxButton* m_SendButton;

        void SendMessage(const std::string_view&);
    public:
        MainUI();

        void OnConnectButton(wxCommandEvent&);
        void OnDisconnectButton(wxCommandEvent&);

        void OnTextInput(wxCommandEvent&);
        void OnSendMessage(wxCommandEvent&);

        void OnConnect(std::optional<std::string>) override;
        void OnChatMessage(std::string) override;
        void OnDisconnect(std::optional<std::string>) override;
    };
}

#endif