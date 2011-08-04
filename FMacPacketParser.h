#ifndef FMACPACKETPARSER_H
#define FMACPACKETPARSER_H

#include <QObject>
#include <QList>
#include <stdint.h>

struct NodeData                   //所有的NodeData都會有以下成員變數
{
	uint8_t senderNodeId;
	uint8_t dataSourceNodeId;
	uint8_t dataSourceTier;
	uint16_t temperature;
	uint16_t humidity;
	uint16_t pest;
	uint32_t par;
};

class FMacPacketParser : public QObject
{
    Q_OBJECT

public:
    explicit FMacPacketParser(QObject *parent = 0);

signals:
	void gotMaxTier(int tier);
	void gotNodePath(int nodeId, int parentId, bool relayed = false);
	void gotData(NodeData data, bool isSupplemental = false);
	void hadAwakeNode();
	void shouldReroute();

public slots:
	void processPacket(QList<uint8_t> packet);

private:
	NodeData getDataOfPacket(QList<uint8_t> packet);

};

#endif // FMACPACKETPARSER_H
