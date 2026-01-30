#include "EmotionLabelItem.h"
#include <QMovie>

EmotionLabelItem::EmotionLabelItem(QWidget* parent)
	: QClickLabel(parent)
{
	initControl();
	// ??????ï…???????
	connect(this, &QClickLabel::clicked, [this]
		{
			emit emotionClicked(m_emotionName);
		});

}

EmotionLabelItem::~EmotionLabelItem()
{
}

void EmotionLabelItem::setEmotionName(int emotionName)
{
	m_emotionName = emotionName;
	QString imageName = QString(":/Resources/MainWindow/emotion/%1.png").arg(emotionName);

	// ?????????????????? QMovie
	// QMovie??¡¤?????????????;
	m_apngMovie = new QMovie(imageName, "apng", this);

	m_apngMovie->start();			// ???
	m_apngMovie->stop();			// ???
	setMovie(m_apngMovie);		// ???????????????????


}

void EmotionLabelItem::initControl()
{
	setAlignment(Qt::AlignCenter);						// ???§Ø???
	setObjectName("emotionLabelItem");			// ?????????
	setFixedSize(32, 32);

}
