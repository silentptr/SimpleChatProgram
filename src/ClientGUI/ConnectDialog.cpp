#include "SCP/ClientGUI/ConnectDialog.h"

namespace SCP::ClientGUI
{
    ConnectDialog::ConnectDialog(wxWindow* parent) :
    wxDialog(parent, wxID_ANY, "Connect", wxDefaultPosition, wxSize(400, 250))
    {
        wxPanel* panel = new wxPanel(this);
        wxBoxSizer* vertSizer = new wxBoxSizer(wxVERTICAL);
        panel->SetSizer(vertSizer);

        m_IP = new wxTextCtrl(panel, wxID_ANY);
        m_Port = new wxTextCtrl(panel, wxID_ANY);
        m_Username = new wxTextCtrl(panel, wxID_ANY);

        m_Cancel = new wxButton(panel, SCP_CONNECTDIALOG_CANCEL, "Cancel");
        m_Connect = new wxButton(panel, SCP_CONNECTDIALOG_CONNECT, "Connect");

        wxBoxSizer* row1 = new wxBoxSizer(wxHORIZONTAL);
        row1->Add(new wxStaticText(panel, wxID_ANY, "IP:"), 0, wxLEFT | wxUP | wxDOWN, 6);
        row1->Add(m_IP, 1, wxEXPAND | wxALL, 6);

        wxBoxSizer* row2 = new wxBoxSizer(wxHORIZONTAL);
        row2->Add(new wxStaticText(panel, wxID_ANY, "Port:"), 0, wxLEFT | wxUP | wxDOWN, 6);
        row2->Add(m_Port, 1, wxEXPAND | wxALL, 6);

        wxBoxSizer* row3 = new wxBoxSizer(wxHORIZONTAL);
        row3->Add(new wxStaticText(panel, wxID_ANY, "Username:"), 0, wxLEFT | wxUP | wxDOWN, 6);
        row3->Add(m_Username, 1, wxEXPAND | wxALL, 6);

        wxBoxSizer* row4 = new wxBoxSizer(wxHORIZONTAL);
        row4->Add(m_Cancel, 0, wxALL, 6);
        row4->Add(m_Connect, 0, wxALL, 6);

        vertSizer->Add(row1, 0, wxEXPAND);
        vertSizer->Add(row2, 0, wxEXPAND);
        vertSizer->Add(row3, 0, wxEXPAND);
        vertSizer->Add(row4, 0, wxALIGN_CENTER);

        Bind(wxEVT_BUTTON, &ConnectDialog::OnCancel, this, SCP_CONNECTDIALOG_CANCEL);
        Bind(wxEVT_BUTTON, &ConnectDialog::OnConnect, this, SCP_CONNECTDIALOG_CONNECT);

        CenterOnParent();
    }

    void ConnectDialog::OnCancel(wxCommandEvent&)
    {
        m_IP->Clear();
        Close();
    }

    void ConnectDialog::OnConnect(wxCommandEvent&)
    {
        Close();
    }

    std::optional<ConnectInfo> ConnectDialog::GetConnectInfo()
    {
        if (m_IP->IsEmpty()) { return std::nullopt; }
        
        std::string ip = m_IP->GetValue().ToStdString();

        if (ip.length() < 3)
        {
            return std::nullopt;
        }

        std::uint16_t port;
        unsigned long l;

        try
        {
            l = std::stoul(m_Port->GetValue().ToStdString());
        }
        catch (...)
        {
            return std::nullopt;
        }
        
        if (l > 65535) { return std::nullopt; }

        port = static_cast<std::uint16_t>(l);

        std::string username = m_Username->GetValue().ToStdString();

        if (username.length() < 3 || username.length() > 30)
        {
            return std::nullopt;
        }

        return std::optional<ConnectInfo>({std::move(ip), port, std::move(username)});
    }
}