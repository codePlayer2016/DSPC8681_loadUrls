/*
 * loadUrl.c
 *
 *  Created on: Dec 10, 2015
 *      Author: root
 */

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<unistd.h>
#include <string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <sys/mman.h>
#include <sys/time.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<string.h>
#include"DPU_ioctl.h"
#include "LinkLayer.h"

#define IMG_SIZE4_DSP (0x100000)
#define PAGE_SIZE (0x1000)
#define WAITTIME (0x07FFFFFF)
#define WTBUFLENGTH (2*4*1024)
#define RDBUFLENGTH (4*1024*1024-4*4*1024)
#define URL_ITEM_SIZE (102)

typedef struct _tagArguments
{
	char *pUrlListPath;
	char *pDevicePath;
} Arguments;

void showHelp(int retVal);
void showError(int retVal);
int parseArguments(int argc, char **argv, Arguments* pArguments);
int loadUrl(Arguments* pArguments);
int getUrlList(FILE *fpUrlList, char *pArrayUrlList, int *pUrlItmeNum);

int main(int argc, char **argv)
{
	// TODO: -f <url list file>
	// TODO: -t <device>
	// TODO: --help
	int retVal = 0;
	Arguments arguments;

	retVal = parseArguments(argc, argv, &arguments);
	if (retVal == 0)
	{
		retVal = loadUrl((Arguments *) &arguments);
		showError(retVal);
	}
	else
	{
		showHelp(retVal);
	}

	return (retVal);
}

void showHelp(int retVal)
{
	printf("--help\n");
	printf("input the command as follow: ");
	printf("load-urls -f <url-list> -t <device>\n");
	printf("-t <device>\n");
	printf("\t specify the target device\n");
	printf("-f <url-list>\n");
	printf("\t input the url-list file\n");
}

void showError(int retVal)
{

}

int parseArguments(int argc, char **argv, Arguments* pArguments)
{
	int retVal = 0;
	if (argc == 5)
	{
		if (strcasecmp("-f", argv[1]) == 0)
		{
			pArguments->pUrlListPath = argv[2];
			//printf("the input file is %s\n", pArguments->pUrlListPath);
		}
		else
		{
			retVal = -1;
			printf("error:input file is %s\n", argv[2]);
		}

		if ((retVal == 0) && (pArguments->pUrlListPath != NULL))
		{
			if (strcasecmp("-t", argv[3]) == 0)
			{
				pArguments->pDevicePath = argv[4];
				//printf("the device is %s\n", pArguments->pDevicePath);
			}
			else
			{
				retVal = -2;
				printf("error:the device is %s\n", argv[4]);
			}
		}

	}
	else
	{
		printf("error:the para number is %d\n", argc);
		retVal = -3;
	}
	if (retVal != 0)
	{
		showHelp(retVal);
	}
	return (retVal);

#if 0
	pArguments->pUrlListPath = "/home/hawke/urllist.txt";
	pArguments->pDevicePath = "dpu0";

	printf("the urlList file is %s\n", pArguments->pUrlListPath);
	return (retVal);
#endif
}

int loadUrl(Arguments* pArguments)
{
	//hello
	int retVal = 0;
	int retIoVal = 0;
	float timeElapse = 0;

	struct timeval downloadStart;
	struct timeval downloadEnd;
	int fdDevice = 0;
	FILE *fpUrlList = NULL;
	const char dev_name[] = "/dev/DPU_driver_linux";
	char arrayUrlList[50 * 102];
	char *pArrayUList = arrayUrlList;
	int urlItmeNum = 0;
	int wtConfig = 0;
	uint32_t *g_pMmapAddr = NULL;
	uint32_t mmapAddrLength = (4 * 1024 * 1024);
	DPUDriver_WaitBufferReadyParam waitWriteBufferReadyParam;
	int status = -1;
	uint32_t *pUrlNums = NULL;
	uint32_t *pDownloadPicNums = NULL;
	uint32_t *pFailedPicNUms = NULL;
	uint32_t downloadPicNums = 0;
	uint32_t failLoadPicNums = 0;

	LinkLayerBuffer *pLinkLayerBuffer = (LinkLayerBuffer *) malloc(
			sizeof(LinkLayerBuffer));

	if (pLinkLayerBuffer != NULL)
	{
		pLinkLayerBuffer->inBufferLength = RDBUFLENGTH; //
		pLinkLayerBuffer->outBufferLength = WTBUFLENGTH; //8k
	}
	else
	{
		retVal = -1;
	}
	// fopen the device.
	if (retVal == 0)
	{
		fdDevice = open(dev_name, O_RDWR);
		if (fdDevice >= 0)
		{
			//printf("the open device is %s\n", dev_name);
		}
		else
		{
			printf("pcie device open error\n");
			retVal = -2;
		}
	}
	// get the url from the urlList.txt.
	if (retVal == 0)
	{
		fpUrlList = fopen(pArguments->pUrlListPath, "rb");
		if (fpUrlList != NULL)
		{

		}
		else
		{
			printf("open the %s failed\n", pArguments->pUrlListPath);
			retVal = -3;
		}
	}

	if (retVal == 0)
	{
		retVal = getUrlList(fpUrlList, pArrayUList, &urlItmeNum);
		if (retVal == 0)
		{
			fclose(fpUrlList);
		}
		else
		{
			printf("get url from urlList File failed\n");
			retVal = -4;
		}
	}
	// mmap and get the registers.
	if (retVal == 0)
	{
		g_pMmapAddr = (uint32_t *) mmap(NULL, mmapAddrLength,
				PROT_READ | PROT_WRITE, MAP_SHARED, fdDevice, 0);
		// polling the dsp can be written to.
		if ((int) g_pMmapAddr != -1)
		{
			//printf("mmap finished\n");
			pLinkLayerBuffer->pOutBuffer = (uint32_t *) ((uint8_t *) g_pMmapAddr
					+ PAGE_SIZE * 2);
			pLinkLayerBuffer->pInBuffer = (uint32_t *) ((uint8_t *) g_pMmapAddr
					+ PAGE_SIZE * 2 * 2);

			pUrlNums = (uint32_t *) ((uint8_t *) g_pMmapAddr + PAGE_SIZE
					+ 3 * sizeof(4));
			pDownloadPicNums = (uint32_t *) ((uint8_t *) g_pMmapAddr
					+ 3 * sizeof(4));

			pFailedPicNUms = (uint32_t *) ((uint8_t *) g_pMmapAddr
					+ 4 * sizeof(4));
//			printf(
//					"g_pMmapAddr=0x%x,pDownloadPicNums=0x%x,pDownloadPicNums=0x%x\n",
//					g_pMmapAddr, pDownloadPicNums, pFailedPicNUms);

			waitWriteBufferReadyParam.waitType = LINKLAYER_IO_WRITE;
			waitWriteBufferReadyParam.pendTime = WAITTIME;
			waitWriteBufferReadyParam.pBufStatus = &status;
			retIoVal = ioctl(fdDevice, DPU_IO_CMD_WAITBUFFERREADY,
					&waitWriteBufferReadyParam); // dsp should init the RD register to empty in DSP.
		}
		else
		{
			printf("mmap failed\n");
			retVal = -5;
		}
	}

	// write the url to output zone.write the urlNum to output zone.
	if (retVal == 0)
	{
		if (retIoVal != -1)
		{
			//printf("ioctl for waitWriteBuffer ready finished\n");
			if (status == 0)
			{
				//printf("writeBuffer ready\n");
				memcpy(pUrlNums, &urlItmeNum, sizeof(int));
				memcpy(pLinkLayerBuffer->pOutBuffer, arrayUrlList,
						(urlItmeNum * URL_ITEM_SIZE));
				// NOTE: need this.
				printf("loading list to dpu0 ...\n");
			}
			else
			{
				printf("wait writeBuffer timeout\n");
				retVal = -6;
			}
		}
		else
		{
			printf("ioctl for waitWriteBuffer ready failed\n");
			retVal = -7;
		}
	}

	// set the pc can be written.
	if (retVal == 0)
	{
		int rdConfig = LINKLAYER_IO_READ;
		retIoVal = ioctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &rdConfig);
		if (retIoVal != -1)
		{
			gettimeofday(&downloadStart, NULL);
			wtConfig = LINKLAYER_IO_WRITE;
			//printf("pc write over\n");
			//pc change the wt reg and dsp's rd reg status can be read in dsp
			retIoVal = ioctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS,
					&wtConfig);
		}
		else
		{
			retVal = -8;
		}
	}

	// TODO should change .pc should not read the picture downloaded by the DSP.
	// polling the RD status register ((DSP read the url over and download picture over(DSP change the wt in dsp)) or not).
	// means the url can be overwrite in the dsp zone.,the pc can write agian.
	if (retVal == 0)
	{
		if (retIoVal != -1)
		{
//			printf(
//					"change the RD status register in dsp by WT ctl register in PC\n");
			waitWriteBufferReadyParam.waitType = LINKLAYER_IO_READ;
			waitWriteBufferReadyParam.pendTime = WAITTIME;
			waitWriteBufferReadyParam.pBufStatus = &status;
			// dsp change the wt ctl reg means pc can read.()
			retIoVal = ioctl(fdDevice, DPU_IO_CMD_WAITBUFFERREADY,
					&waitWriteBufferReadyParam);
		}
		else
		{
			printf("ioctl for changeWriteBuffer status failed\n");
			retVal = -9;

		}
	}

	if (retVal == 0)
	{
		if (retIoVal != -1)
		{
			//printf("ioctl for polling RD status register finished\n");
			if (status == 0)
			{
				gettimeofday(&downloadEnd, NULL);
				timeElapse = ((downloadEnd.tv_sec - downloadStart.tv_sec)
						* 1000000
						+ (downloadEnd.tv_usec - downloadStart.tv_usec));
				//printf("DSP download url finished\n");
				// NOTE: dsp finished the download.wait for pc reading.can add read information code here.
				// NOTE: init some parameters in the dsp-side.

				// read the download status.
				downloadPicNums = *pDownloadPicNums;
				failLoadPicNums = *pFailedPicNUms;

				// init the urlNums to DSP
				*pUrlNums = 0;

				// pc read the finished.single the DSP.
				int wtConfig = LINKLAYER_IO_READ_FIN;
				retIoVal = ioctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS,
						&wtConfig);
			}
			else
			{
				printf("wait readBuffer status timeout\n");
				retVal = -10;
			}
		}
		else
		{
			retVal = -11;
			printf("ioctl for waitReadBuffer status failed\n");
		}
	}

	if (retVal == 0)
	{
		printf("done (%d loaded, %d failed, %f ms elapsed).\n", downloadPicNums,
				failLoadPicNums, (timeElapse / 1000));
		// init for the next instance.
		failLoadPicNums = 0;
		downloadPicNums = 0;
	}
	else
	{
	}

	// one com finished .init the register.
	*pUrlNums = 0;
	wtConfig = LINKLAYER_IO_WRITE_FIN;
	retIoVal = ioctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &wtConfig);

	// release the resource.
	munmap(g_pMmapAddr, mmapAddrLength);
	free(pLinkLayerBuffer);
	close(fdDevice);
	return (retVal);
}
int getUrlList(FILE *fpUrlList, char *pUrlList, int *pUrlItmeNum)
{
	int retVal = 0;
	char urlItem[256];
	char *pUrlItem = urlItem;
	char *pArrayUrlList = pUrlList;
	int enterCharPos = 0;
	int urlItemNum = 0;
	while (fgets(pUrlItem, 256, fpUrlList) != NULL)
	{
		if (strlen(pUrlItem) > 20)
		{
			enterCharPos = 0;
			enterCharPos = (strlen(pUrlItem) - strlen("\n"));
			pUrlItem[enterCharPos] = '\0';
			memcpy(pArrayUrlList, pUrlItem, enterCharPos);
			//printf("the url is %s\n", pArrayUrlList);
			pArrayUrlList += URL_ITEM_SIZE;
			urlItemNum++;
		}
		else
		{
			continue;
		}
	}
	*pUrlItmeNum = urlItemNum;
	//printf("the url item Num is %d\n", *pUrlItmeNum);
	return (retVal);
}
