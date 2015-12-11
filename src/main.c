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
	pArguments->pUrlListPath = "/home/hawke/urllist.txt";
	pArguments->pDevicePath = "dpu0";

	printf("the urlList file is %s\n", pArguments->pUrlListPath);
	return (retVal);
}

int loadUrl(Arguments* pArguments)
{
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
	uint32_t *g_pMmapAddr = NULL;
	uint32_t mmapAddrLength = (4 * 1024 * 1024);
	DPUDriver_WaitBufferReadyParam waitWriteBufferReadyParam;
	int status = -1;

	LinkLayerBuffer *pLinkLayerBuffer = (LinkLayerBuffer *) malloc(
			sizeof(LinkLayerBuffer));

	// fopen the device.
	if (pLinkLayerBuffer != NULL)
	{
		pLinkLayerBuffer->inBufferLength = RDBUFLENGTH; //
		pLinkLayerBuffer->outBufferLength = WTBUFLENGTH; //8k
		fdDevice = open(dev_name, O_RDWR);
	}
	else
	{
		retVal = -1;
	}

	// get the url from the urlList.txt.
	if (fdDevice >= 0)
	{
		printf("the open device is %s\n", dev_name);
		fpUrlList = fopen(pArguments->pUrlListPath, "rb");
	}
	else
	{
		printf("pcie device open error\n");
		retVal = -2;
	}
	if (fpUrlList != NULL)
	{
		retVal = getUrlList(fpUrlList, pArrayUList, &urlItmeNum);
	}
	else
	{
		printf("open the %s failed\n", pArguments->pUrlListPath);
		retVal = -3;
	}

	// mmap and get the registers.
	if (retVal == 0)
	{
		fclose(fpUrlList);
		g_pMmapAddr = (uint32_t *) mmap(NULL, mmapAddrLength,
		PROT_READ | PROT_WRITE, MAP_SHARED, fdDevice, 0);
	}
	else
	{
		printf("get url from urlList File failed\n");
		retVal = -4;
	}

	// polling the WT status register (can write or not).
	if ((int) g_pMmapAddr != -1)
	{
		printf("mmap finished\n");
		pLinkLayerBuffer->pOutBuffer = (uint32_t *) ((uint8_t *) g_pMmapAddr
				+ PAGE_SIZE * 2);
		pLinkLayerBuffer->pInBuffer = (uint32_t *) ((uint8_t *) g_pMmapAddr
				+ PAGE_SIZE * 2 * 2);
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

	// write the url to output zone.
	if (retIoVal != -1)
	{
		printf("ioctl for waitWriteBuffer ready finished\n");
		if (status == 0)
		{
			printf("writeBuffer ready\n");
			memcpy(pLinkLayerBuffer->pOutBuffer, arrayUrlList,
					(urlItmeNum * URL_ITEM_SIZE));
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

	// change the WT ctl register (pc write over and dsp can read).
	if ((retIoVal != -1) && (status == 0))
	{
		gettimeofday(&downloadStart, NULL);
		int wtConfig = LINKLAYER_IO_WRITE;
		//pc change the wt reg and dsp's rd reg status can be read in dsp
		retIoVal = ioctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &wtConfig);
	}
	else
	{
	}

	// TODO should change .pc should not read the picture downloaded by the DSP.
	// polling the RD status register ((DSP read the url over and download picture over(DSP change the wt in dsp)) or not).
	// means the url can be overwrite in the dsp zone.,the pc can write agian.
	if (retIoVal != -1)
	{
		printf(
				"change the RD status register in dsp by WT ctl register in PC\n");
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
		retVal = -8;

	}

	if (retIoVal != -1)
	{
		printf("ioctl for polling RD status register finished\n");
		if (status == 0)
		{
			gettimeofday(&downloadEnd, NULL);
			timeElapse = ((downloadEnd.tv_sec - downloadStart.tv_sec) * 1000000
					+ (downloadEnd.tv_usec - downloadStart.tv_usec));
			printf("DSP download url finished\n");
			// TODO: get the information of the download status informations.
			// TODO: display the download status informations.
		}
		else
		{
			printf("wait readBuffer status timeout\n");
			retVal = -9;
		}
	}
	else
	{
		printf("ioctl for waitReadBuffer status failed\n");
	}

	// TODO: release the resource.
	// release the resource

	printf("loading list to dpu0 ...\n");
	printf("done (1 loaded, 0 failed, %f ms elapsed).\n", (timeElapse / 1000));
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
			printf("the url is %s\n", pArrayUrlList);
			pArrayUrlList += URL_ITEM_SIZE;
			urlItemNum++;
		}
		else
		{
			continue;
		}
	}
	*pUrlItmeNum = urlItemNum;
	printf("the url item Num is %d\n", *pUrlItmeNum);
	return (retVal);
}
