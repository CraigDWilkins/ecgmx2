/*
 * This file is part of MobilECG, an open source clinical grade Holter
 * ECG. For more information visit http://mobilecg.hu
 *
 * Copyright (C) 2016  Robert Csordas, Peter Isza
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "ECGSender.h"
#include "ADS1298.h"
#include "ecg.h"

//#define DEBUG

const int ECGSender::ECG_CHOP_BITS = 5;

ECGSender::ECGSender(Packetizer &iPacketizer):
	compressFifo((char*)compressBuffer, ECG_COMPRESS_OUTPUT_BUFFER_SIZE),
	compressor(compressFifo, ecgPredictor),
	testGenerator(15000,500)
{
	packetizer = &iPacketizer;
	testSignal = false;

	currFrequency = 488.28125f;
}

ECGSender::~ECGSender() {
}

void ECGSender::send(UART_HandleTypeDef *phuart2) {
	//Serial *pc){
	//Create header and calculate data size
	uint8_t header[Packetizer::HEADER_SIZE + sizeof(ECGHeader)];
	ECGHeader *ecgHeader = (ECGHeader *)(header + Packetizer::HEADER_SIZE);

	ecgHeader->lsbInMv = ADS1298::instance().getLsbInMv() * (1 << ECG_CHOP_BITS);
	ecgHeader->samplingFrequency = currFrequency;
	ecgHeader->channelCount = ADS1298::instance().getActiveChannels();

	uint32_t size = ADS1298::instance().getAvailableData();
	uint32_t blockSize = (ecgHeader->channelCount+1)*3;

	if (size>ECG_MAX_SEND_SIZE)
		size=ECG_MAX_SEND_SIZE;

	ecgHeader->sampleCount = size / blockSize;

	if (size==0){
		return;
	}

	//Initialize the compressor
	compressFifo.reset();
	ecgPredictor.reset();
	ecgPredictor.setNumChannels(ecgHeader->channelCount);
	compressor.setNumChannels(ecgHeader->channelCount);

	ADS1298 &ecg=ADS1298::instance();
	//Compress
	for (unsigned pos=0; pos<size; pos+=blockSize){
		ecg.getSample(sampleOfChannels);

		if (testSignal){
			for (int a=0; a<ecgHeader->channelCount; a++){
				sampleOfChannels[a]=testGenerator.getSample(testGenerator.getPeriod()*a/ecgHeader->channelCount);
				testGenerator.next();
			}
		}

		for (int a=0; a<ecgHeader->channelCount; a++){
			sampleOfChannels[a] >>= ECG_CHOP_BITS;
		}
		//Compress
		compressor.putSample(sampleOfChannels);
	}

	ecgHeader->numBits = compressFifo.getAvailableBits();
	size = (ecgHeader->numBits+7) / 8;
	#ifdef DEBUG
	//	pc->printf("Sending packet.\n");
	PrintText("Sending packet.\n");
	#endif

	//Send header
	packetizer->startPacket(header, Packetizer::ECG, (uint16_t)(size+sizeof(ECGHeader)));
	packetizer->checksumBlock((uint8_t*)ecgHeader, sizeof(ECGHeader));
	#ifdef DEBUG
   // 	pc->printf("Packet size: %d \n", (Packetizer::HEADER_SIZE + sizeof(ECGHeader)));
	PrintTextInt("Packet size : ", (Packetizer::HEADER_SIZE + sizeof(ECGHeader)));
	PrintText("\n");
    #endif

    for (unsigned int i = 0; i < (Packetizer::HEADER_SIZE + sizeof(ECGHeader)); i++)
    {
       // pc->putc(header[i]);
	    PrintC(header[i]);
    }

	//Calculate checksum over compressBuffer and add to final checksum value
	packetizer->checksumBlock(compressBuffer, size);
	#ifdef DEBUG
	//	pc->printf("\nCompressBuffer Size: %d\n", size);
	PrintTextInt("\nCompressBuffer Size: ", size);
	PrintText("\n");
	#endif

    for (unsigned int i = 0; i < size; i++)
    {
       // pc->putc(compressBuffer[i]);
	    PrintC(compressBuffer[i]);
	}
	
	//Send checksum
	Packetizer::Checksum chksum = packetizer->getChecksum();
	#ifdef DEBUG
		//pc->printf("\nCompressBuffer Size: %d\n", size);
	PrintTextInt("\nCompressBuffer Size: ", size);
	PrintText("\n");
	#endif

    for (unsigned int i = 0; i < sizeof(chksum); i++)
    {
	    // pc->putc((char)((chksum >> (8*i)) & 0x00FF));
		 char c = (char)((chksum >> (8*i)) & 0x00FF);
	    PrintC(c);
	}
	
	#ifdef DEBUG
    //	pc->printf("Size: %d\n", size); 
	PrintTextInt("Size: ", size);
	PrintText("\n");
    //	pc->printf("Length: %d\n", (uint16_t)(size+sizeof(ECGHeader)));
	PrintTextInt("Length: ", (uint16_t)(size + sizeof(ECGHeader)));
	PrintText("\n");
    //	pc->printf("Checksum is %d\n", chksum);
	PrintTextInt("Checksum is ", chksum);
	PrintText("\n");
    #endif
}
