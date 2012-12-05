#include <QtConcurrentRun>
#include <QPainter>
#include <QProcess>
#include <QTextOption>
#include "TextureDialog/TextureConvertor.h"
#include "SceneEditor/PVRConverter.h"

#include "Platform/Qt/QtLayer.h"

TextureConvertor::TextureConvertor()
	: curJobOriginal(NULL)
{
	// slots will be called in connector(this) thread
	QObject::connect(&loadOriginalWatcher, SIGNAL(finished()), this, SLOT(threadOriginalFinished()), Qt::QueuedConnection);

	for(int i = 0; i < CONVERT_JOB_COUNT; ++i)
	{
		curJobConvert[i] = NULL;

		// slots will be called in connector(this) thread
		QObject::connect(&convertWatcher[i], SIGNAL(finished()), this, SLOT(threadConvertFinished()), Qt::QueuedConnection);
	}
}

void TextureConvertor::getPVR(const DAVA::TextureDescriptor *descriptor, bool forceConver /*= false*/ )
{
	if(NULL != descriptor)
	{
		JobItem newJob;
		newJob.type = JobItem::JobPVR;
		newJob.descriptor = descriptor;
		newJob.forceConvert = forceConver;

		jobStackConvert.push(newJob);
		jobRunNextConvert();
	}
}

void TextureConvertor::getDXT(const DAVA::TextureDescriptor *descriptor, bool forceConver /*= false*/ )
{
	if(NULL != descriptor)
	{
		JobItem newJob;
		newJob.type = JobItem::JobDXT;
		newJob.descriptor = descriptor;
		newJob.forceConvert = forceConver;

		jobStackConvert.push(newJob);
		jobRunNextConvert();
	}
}

void TextureConvertor::loadOriginal(const DAVA::TextureDescriptor *descriptor)
{
	if(NULL != descriptor)
	{
		// we dont care about job-type and descriptor when starting job to load original texture
		JobItem newJob;
		newJob.descriptor = descriptor;

		jobStackOriginal.push(newJob);
		jobRunNextOriginal();
	}
}

void TextureConvertor::jobRunNextConvert()
{
	int freeIndex = jobGetConvertFreeIndex();

	// if there is no already running work
	if(-1 != freeIndex)
	{
		// get the new work
		curJobConvert[freeIndex] = jobStackConvert.pop();
		if(NULL != curJobConvert[freeIndex])
		{
			curJobConvert[freeIndex]->descriptorCopy = *curJobConvert[freeIndex]->descriptor;

			switch(curJobConvert[freeIndex]->type)
			{
			case JobItem::JobPVR:
				{
					QFuture<QImage> f = QtConcurrent::run(this, &TextureConvertor::convertThreadPVR, curJobConvert[freeIndex]);
					convertWatcher[freeIndex].setFuture(f);
				}
				break;
			case JobItem::JobDXT:
				{
					QFuture<QImage> f = QtConcurrent::run(this, &TextureConvertor::convertThreadDXT, curJobConvert[freeIndex]);
					convertWatcher[freeIndex].setFuture(f);
				}
				break;
			default:
				break;
			}
		}

		emit convertStatus(curJobConvert[freeIndex], jobStackConvert.size());
	}
}

void TextureConvertor::jobRunNextOriginal()
{
	// if there is no already running work
	if((loadOriginalWatcher.isFinished() || loadOriginalWatcher.isCanceled()) && NULL == curJobOriginal)
	{
		// get the new work
		curJobOriginal = jobStackOriginal.pop();
		if(NULL != curJobOriginal)
		{
			QFuture<QImage> f = QtConcurrent::run(this, &TextureConvertor::loadOriginalThread, curJobOriginal);
			loadOriginalWatcher.setFuture(f);
		}
	}
}

QImage TextureConvertor::loadOriginalThread(JobItem *item)
{
	QImage img;

    void *pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();
    
	if(NULL != item && NULL != item->descriptor)
	{
		img = QImage(item->descriptor->GetSourceTexturePathname().c_str());
	}

    DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
	return img;
}

QImage TextureConvertor::convertThreadPVR(JobItem *item)
{
	QImage qtImage;

    void *pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();

	if(NULL != item && item->descriptorCopy.pvrCompression.format != DAVA::FORMAT_INVALID)
	{
		DAVA::String sourcePath = item->descriptorCopy.GetSourceTexturePathname();
		DAVA::String outputPath = PVRConverter::Instance()->GetPVRToolOutput(sourcePath);

		if(!outputPath.empty())
		{
			if(item->forceConvert || !DAVA::FileSystem::Instance()->IsFile(outputPath))
			{
				QString command = DAVA::FileSystem::Instance()->GetCurrentWorkingDirectory().c_str();

				command += "/";
				command += PVRConverter::Instance()->GetCommandLinePVR(sourcePath, item->descriptorCopy).c_str();
				DAVA::Logger::Info("%s", command.toStdString().c_str());

				QProcess p;
				p.start(command);
				p.waitForFinished(-1);

				if(QProcess::NormalExit != p.exitStatus())
				{
					DAVA::Logger::Error("Convertor process crushed");
				}
				if(0 != p.exitCode())
				{
					DAVA::Logger::Error("Convertor exit with error %d", p.exitCode());
					DAVA::Logger::Error("Stderror:\n%s", p.readAllStandardError().constData());
					DAVA::Logger::Error("Stdout:\n%s", p.readAllStandardOutput().constData());
					DAVA::Logger::Error("---");
				}
			}

			item->descriptorCopy.UpdateDateAndCrcForFormat(DAVA::PVR_FILE);
			item->descriptorCopy.Save();

			std::vector<DAVA::Image *> davaImages = DAVA::ImageLoader::CreateFromFile(outputPath);

			if(davaImages.size() > 0)
			{
				DAVA::Image *davaImage = davaImages[0];
				qtImage = fromDavaImage(davaImage);
			}

			for_each(davaImages.begin(), davaImages.end(),  DAVA::SafeRelease< DAVA::Image>);
		}
	}
	else
	{
		QRect r(0, 0, 200, 200);
		qtImage = QImage(r.size(), QImage::Format_ARGB32);

		QFont font;
		font.setPointSize(18);

		QPainter p(&qtImage);
        p.setBrush(QBrush(QColor(0, 0, 0, 0)));
        p.setPen(QColor(200, 0, 0));
		p.setFont(font);

		p.drawText(r, "Wrong PVR format", QTextOption(Qt::AlignCenter));
	}

    DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
	return qtImage;
}

QImage TextureConvertor::convertThreadDXT(JobItem *item)
{
	QImage convertedImage;

    void *pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();

    
	// TODO:
	// convert
	// ...
	if(NULL != item && item->descriptorCopy.dxtCompression.format != DAVA::FORMAT_INVALID)
	{
		
		item->descriptorCopy.UpdateDateAndCrcForFormat(DAVA::DXT_FILE);
		item->descriptorCopy.Save();

		DAVA::String sourcePath = item->descriptorCopy.GetSourceTexturePathname();
		
		std::vector<DAVA::Image *> davaImages = DAVA::ImageLoader::CreateFromFile(sourcePath);

		if(davaImages.size() > 0)
		{
			DAVA::Image *davaImage = davaImages[0];
			convertedImage = fromDavaImage(davaImage);
		}

		for_each(davaImages.begin(), davaImages.end(),  DAVA::SafeRelease< DAVA::Image>);
		
	}
	else
	{
		QRect r(0, 0, 200, 200);
		convertedImage = QImage(r.size(), QImage::Format_ARGB32);

		QFont font;
		font.setPointSize(18);

		QPainter p(&convertedImage);
		p.setBrush(QColor(0,255,0));
		p.setPen(QColor(155, 0, 0));
		p.setFont(font);

		p.drawText(r, "DXT isn't supported", QTextOption(Qt::AlignCenter));
	}

    DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
	return convertedImage;
}

void TextureConvertor::threadOriginalFinished()
{
	if(loadOriginalWatcher.isFinished() && NULL != curJobOriginal)
	{
		emit readyOriginal(curJobOriginal->descriptor, loadOriginalWatcher.result());

		DAVA::Logger::Info("%s loaded", curJobOriginal->descriptor->pathname.c_str());

		delete curJobOriginal;
		curJobOriginal = NULL;
	}

	jobRunNextOriginal();
}

void TextureConvertor::threadConvertFinished()
{
	int doneIndex = jobGetConvertIndex((QFutureWatcher<QImage> *) QObject::sender());

	if(-1 != doneIndex)
	{
		if(convertWatcher[doneIndex].isFinished() && NULL != curJobConvert[doneIndex])
		{
			switch(curJobConvert[doneIndex]->type)
			{
			case JobItem::JobPVR:
				emit readyPVR(curJobConvert[doneIndex]->descriptor, convertWatcher[doneIndex].result());
				break;
			case JobItem::JobDXT:
				emit readyDXT(curJobConvert[doneIndex]->descriptor, convertWatcher[doneIndex].result());
				break;
			default:
				break;
			}

			delete curJobConvert[doneIndex];
			curJobConvert[doneIndex] = NULL;
		}
	}

	jobRunNextConvert();
}

int TextureConvertor::jobGetConvertFreeIndex()
{
	int index = -1;

	for(int i = 0; i < CONVERT_JOB_COUNT; ++i)
	{
		if((convertWatcher[i].isFinished() || convertWatcher[i].isCanceled()) && NULL == curJobConvert[i])
		{
			index = i;
			break;
		}
	}

	return index;
}

int TextureConvertor::jobGetConvertIndex(QFutureWatcher<QImage> *watcher)
{
	int index = -1;

	for(int i = 0; i < CONVERT_JOB_COUNT; ++i)
	{
		if(watcher == &convertWatcher[i])
		{
			index = i;
			break;
		}
	}

	return index;
}

QImage TextureConvertor::fromDavaImage(DAVA::Image *image)
{
	QImage qtImage;

	if(NULL != image)
	{
		QRgb *line;

		switch(image->format)
		{
		case DAVA::FORMAT_PVR4:
		case DAVA::FORMAT_PVR2:
		case DAVA::FORMAT_RGBA8888:
			{
				DAVA::uint32 *data = (DAVA::uint32 *) image->data;
				DAVA::uint32 c;

				qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);

				// convert DAVA:RGBA8888 into Qt ARGB8888
				for (int y = 0; y < image->height; y++) 
				{
					line = (QRgb *) qtImage.scanLine(y);
					for (int x = 0; x < image->width; x++) 
					{
						c = data[y * image->width + x];
						line[x] = (c & 0xFF00FF00) | ((c & 0x00FF0000) >> 16) | ((c & 0x000000FF) << 16);
					}
				}
			}
			break;

		case DAVA::FORMAT_RGBA5551:
			{
				DAVA::uint16 *data = (DAVA::uint16 *) image->data;
				DAVA::uint32 c;

				qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);

				// convert DAVA:RGBA5551 into Qt ARGB8888
				for (int y = 0; y < image->height; y++) 
				{
					line = (QRgb *) qtImage.scanLine(y);
					for (int x = 0; x < image->width; x++) 
					{
						DAVA::uint32 a;
						DAVA::uint32 r;
						DAVA::uint32 g;
						DAVA::uint32 b;

						c = data[y * image->width + x];
						r = (c >> 11) & 0x1f;
						g = (c >> 6) & 0x1f;
						b = (c >> 1) & 0x1f;
						a = (c & 0x1) ? 0xff000000 : 0x0;

						line[x] = (a | (r << (16 + 3)) | (g << (8 + 3)) | (b << 3));
					}
				}
			}
			break;

		case DAVA::FORMAT_RGBA4444:
			{
				DAVA::uint16 *data = (DAVA::uint16 *) image->data;
				DAVA::uint32 c;

				qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);

				// convert DAVA:RGBA4444 into Qt ARGB8888
				for (int y = 0; y < image->height; y++) 
				{
					line = (QRgb *) qtImage.scanLine(y);
					for (int x = 0; x < image->width; x++) 
					{
						DAVA::uint32 a;
						DAVA::uint32 r;
						DAVA::uint32 g;
						DAVA::uint32 b;

						c = data[y * image->width + x];
						r = (c >> 12) & 0xf;
						g = (c >> 8) & 0xf;
						b = (c >> 4) & 0xf;
						a = (c & 0xf);

						line[x] = ((a << (24 + 4)) | (r << (16 + 4)) | (g << (8+4)) | (b << 4));
					}
				}
			}
			break;

		case DAVA::FORMAT_RGB565:
			{
				DAVA::uint16 *data = (DAVA::uint16 *) image->data;
				DAVA::uint32 c;

				qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);

				// convert DAVA:RGBA565 into Qt ARGB8888
				for (int y = 0; y < image->height; y++) 
				{
					line = (QRgb *) qtImage.scanLine(y);
					for (int x = 0; x < image->width; x++) 
					{
						DAVA::uint32 a;
						DAVA::uint32 r;
						DAVA::uint32 g;
						DAVA::uint32 b;

						c = data[y * image->width + x];
						a = 0xff;
						r = (c >> 11) & 0x1f;
						g = (c >> 5) & 0x3f;
						b = c & 0x1f;

						line[x] = ((a << 24) | (r << (16 + 3)) | (g << (8 + 2)) | (b << 3));
					}
				}
			}
			break;

		case DAVA::FORMAT_A8:
			{
				DAVA::uint8 *data = (DAVA::uint8 *) image->data;
				DAVA::uint32 c;

				qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);

				// convert DAVA:RGBA565 into Qt ARGB8888
				for (int y = 0; y < image->height; y++) 
				{
					line = (QRgb *) qtImage.scanLine(y);
					for (int x = 0; x < image->width; x++) 
					{
						c = data[y * image->width + x];
						line[x] = ((0xff << 24) | (c << 16) | (c << 8) | c);
					}
				}
			}
			break;

		default:
			break;
		}
	}

	return qtImage;
}
