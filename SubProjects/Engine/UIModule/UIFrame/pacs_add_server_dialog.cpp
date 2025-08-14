#include "pacs_add_server_dialog.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>

#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Memory.h"

PACSAddServerDialog::PACSAddServerDialog(QWidget* parent /*= nullptr*/)
	:CW3Dialog("PACS Add Server", parent)
{
	SetLayout();
}

PACSAddServerDialog::~PACSAddServerDialog()
{

}

void PACSAddServerDialog::SetLayout()
{
	m_contentLayout->setContentsMargins(0, 0, 0, 0);
	m_contentLayout->setSpacing(0);

	m_contentLayout->addLayout(CreateTopLayout());
	m_contentLayout->addLayout(CreateBottomLayout());
}

QHBoxLayout* PACSAddServerDialog::CreateTopLayout()
{
	QHBoxLayout* top_layout = new QHBoxLayout();
	{
		top_layout->setSpacing(10);
		top_layout->setMargin(10);

		QVBoxLayout* nickname_layout = new QVBoxLayout();
		{
			QLabel* label_caption = new QLabel(this);
			//label_caption->setText(lang::LanguagePack::txt_total_number_of_selected_images() + " : ");
			label_caption->setText("Nickname");
			label_caption->setAlignment(Qt::AlignLeft);

			nickname_line_edit_ = new QLineEdit(this);
			nickname_layout->addWidget(label_caption);
			nickname_layout->addWidget(nickname_line_edit_);
		}

		QVBoxLayout* server_ae_title_layout = new QVBoxLayout();
		{
			QLabel* label_caption = new QLabel(this);
			label_caption->setText("AE_Title");
			label_caption->setAlignment(Qt::AlignLeft);

			server_ae_title_line_edit_ = new QLineEdit(this);
			server_ae_title_layout->addWidget(label_caption);
			server_ae_title_layout->addWidget(server_ae_title_line_edit_);
		}

		QVBoxLayout* server_ip_layout = new QVBoxLayout();
		{
			QLabel* label_caption = new QLabel(this);
			label_caption->setText("Server_IP");
			label_caption->setAlignment(Qt::AlignLeft);

			server_ip_line_edit_ = new QLineEdit(this);
			QString ip_range = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
			QRegExp ip_regex("^" + ip_range + "\\." + ip_range + "\\." + ip_range + "\\." + ip_range + "$");
			QRegExpValidator* ip_validator = new QRegExpValidator(ip_regex, this);

			server_ip_line_edit_->setValidator(ip_validator);
			server_ip_line_edit_->setInputMask("000.000.000.000");

			server_ip_layout->addWidget(label_caption);
			server_ip_layout->addWidget(server_ip_line_edit_);
		}

		QVBoxLayout* port_layout = new QVBoxLayout();
		{
			QLabel* label_caption = new QLabel(this);
			label_caption->setText("Port");
			label_caption->setAlignment(Qt::AlignLeft);

			port_line_edit_ = new QLineEdit(this);
			QIntValidator* int_validator = new QIntValidator(0, 65535);
			port_line_edit_->setValidator(int_validator);

			port_layout->addWidget(label_caption);
			port_layout->addWidget(port_line_edit_);
		}

		top_layout->addLayout(nickname_layout);
		top_layout->addLayout(server_ae_title_layout);
		top_layout->addLayout(server_ip_layout);
		top_layout->addLayout(port_layout);
	}

	return top_layout;
}

QVBoxLayout* PACSAddServerDialog::CreateBottomLayout()
{
	QVBoxLayout* bottom_layout = new QVBoxLayout();
	{

		QHBoxLayout* button_layout = new QHBoxLayout();
		{
			button_layout->setSpacing(10);
			button_layout->setMargin(10);
			button_layout->setAlignment(Qt::AlignBottom | Qt::AlignRight);

			QToolButton* ok_button = new QToolButton(this);
			QToolButton* cancel_button = new QToolButton(this);

			connect(ok_button, SIGNAL(clicked()), this, SLOT(slotOK()));
			connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

			ok_button->setText(lang::LanguagePack::txt_ok());
			cancel_button->setText(lang::LanguagePack::txt_cancel());

			button_layout->addWidget(ok_button);
			button_layout->addWidget(cancel_button);

			//positive_button_ = ok_button;
		}

		err_msg_ = new QLabel(this);
		err_msg_->setAlignment(Qt::AlignCenter);

		bottom_layout->addWidget(err_msg_);
		bottom_layout->addLayout(button_layout);
	}

	return bottom_layout;
}

void PACSAddServerDialog::slotOK()
{
	QString nickname = nickname_line_edit_->text();
	QString server_ae_title = server_ae_title_line_edit_->text();
	QString server_ip = server_ip_line_edit_->text();
	QString port = port_line_edit_->text();

	if (nickname.isEmpty())
	{
		err_msg_->setText("Error : Nickname is Empty");
		return;
	}
	else
	{
		int size = nickname_list_.size();
		for (int i = 0; i < size; ++i)
		{
			if (nickname.compare(nickname_list_[i], Qt::CaseInsensitive) == 0)
			{
				err_msg_->setText("Error : Nickname is duplicate");
				return;
			}
		}
	}

	if (server_ae_title.isEmpty())
	{
		err_msg_->setText("Error : AE Title is Empty");
		return;
	}

	if (server_ip.compare("...") == 0)
	{
		err_msg_->setText("Error : Server IP is Empty");
		return;
	}
	else
	{
		int size = server_ip.size();
		if (server_ip[size - 1] == '.')
		{
			err_msg_->setText("Error : Server IP is Inappropriate");
			return;
		}

		for (int i = 0; i < size - 1; ++i)
		{
			if (server_ip[i] == '.' && server_ip[i + 1] == '.')
			{
				err_msg_->setText("Error : Server IP is Inappropriate");
				return;
			}
		}
	}

	if (port.isEmpty())
	{
		err_msg_->setText("Error : Port is Empty");
		return;
	}

	emit sigAddPACSServerInfo(nickname, server_ae_title, server_ip, port);
	reject();
}
