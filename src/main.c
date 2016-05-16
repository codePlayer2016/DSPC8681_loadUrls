/*
 * loadUrl.c
 *
 *  Created on: Dec 10, 2015
 *      Author: root
 */

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<sys/select.h>
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
#define URLNUM (4)

typedef struct _tagArguments
{
	char *pUrlListPath;
	char *pDevicePath;
} Arguments;

FILE *fpUrlList = NULL;
//API encapsulation  Vab means video accelarate board

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
int VabOpen(const char* dev_name, int flag)
{
	int FdHandle = 0;
	FdHandle = open(dev_name, flag);
	if (FdHandle >= 0)
	{
		printf("pcie device open success\n");
		return FdHandle;
	}
	else
	{
		printf("pcie device open error\n");
		return FdHandle;
	}
}
uint32_t * VabMmap(void *start, size_t length, int prot, int flags, int fd,
		off_t offsize)
{
	uint32_t MmapAddr = NULL;
	MmapAddr = (uint32_t *) mmap(start, length, prot, flags, fd, offsize);
	if ((int) MmapAddr != -1)
	{
		printf("mmap success!\n");
		return MmapAddr;
	}
	else
	{
		printf("mmap failed\n");
		return MmapAddr;
	}

}
int VabIoctl(int handle, int command, void *args)
{
	int retVal;
	retVal = ioctl(handle, command, args);
	if (retVal != -1)
	{
	}
	else
	{
		printf("ioctl error\n");
	}
	return retVal;
}
int VabRead(int handle, void *in_buffer, int nbyte)
{
	int rd = 0;
	rd = read(handle, in_buffer, nbyte);
	if (rd < 0)
	{
		printf("read error\n");
		return rd;
	}
	else
	{
		return rd;
	}
}
int VabWrite(int handle, void *out_buffer, int nbyte)
{
	int wd = 0;
	wd = write(handle, out_buffer, nbyte);
	if (wd < 0)
	{
		printf("write error\n");
		return wd;
	}
	else
	{
		return wd;
	}
}
off_t VabSeek(int fd, off_t offset, int whence)
{
	int ret;
	ret = lseek(fd, offset, whence);

	if (ret >= 0)
	{
		return ret;
	}
	else
	{
		printf("seek error\n");
		return -1;
	}
}
unsigned int VabSelect(int fd, fd_set*rd, fd_set*wd, fd_set* ed,
		struct timeval*timeout)
{
	fd_set rfds, wfds;
	FD_ZERO(&rfds);
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	FD_SET(fd, &wfds);
	select(fd + 1, &rfds, &wfds, NULL, NULL);
	if (FD_ISSET(fd, &rfds))
	{
		printf("poll monitor:can be read\n");
	}
	if (FD_ISSET(fd, &wfds))
	{
		printf("poll monitor:can be written\n");
	}

}
int VabTrue(int retValue)
{
	if (retValue != -1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
DPUDriver_WaitBufferReadyParam VabParamAssign(int waitType, uint32_t pendTime,
		int32_t * status)
{
	DPUDriver_WaitBufferReadyParam readyParam;
	readyParam.waitType = waitType;
	readyParam.pendTime = pendTime;
	readyParam.pBufStatus = status;
	return readyParam;
}
void VabGetInfo(uint32_t * mmapAddr, uint32_t **urlNums,
		uint32_t **downloadPicNums, uint32_t **failedPicNUms)
{

	*urlNums = (uint32_t *) ((uint8_t *) mmapAddr + PAGE_SIZE + 3 * sizeof(4));

	*downloadPicNums = (uint32_t *) ((uint8_t *) mmapAddr + 3 * sizeof(4));

	*failedPicNUms = (uint32_t *) ((uint8_t *) mmapAddr + 4 * sizeof(4));

}
int VabMunmap(uint32_t *addr, int size)
{
	munmap(addr, size);
}
void VabClose(int handle)
{
	close(handle);
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
	int index = 0;
	if (argc == 5)
	{
		if (strcasecmp("-f", argv[1]) == 0)
		{
			pArguments->pUrlListPath = argv[2];
			printf("the input file is %s\n", pArguments->pUrlListPath);
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
				printf("the device is %s\n", pArguments->pDevicePath);
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
	int retVal = 0;
	int retIoVal = 0;
	float timeElapse = 0;
	struct timeval downloadStart;
	struct timeval downloadEnd;
	int fdDevice = 0;
	//FILE *fpUrlList = NULL;
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
	int downloadPicNums = 0;
	int failLoadPicNums = 0;
	interruptAndPollParam interruptPollParams;
	interruptAndPollParam *pInterruptPollParams =
			(interruptAndPollParam *) &interruptPollParams;


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
		return retVal;
	}
	// fopen the device.
	fdDevice = VabOpen(dev_name, O_RDWR);
	if (fdDevice < 0)
	{
		fdDevice = -2;
		return fdDevice;

	}
	if (fpUrlList == NULL)
	{
		// get the url from the urlList.txt.
		fpUrlList = fopen(pArguments->pUrlListPath, "rb");
		if (fpUrlList != NULL)
		{
			printf("open file success!\n");
		}
		else
		{
			printf("open the %s failed\n", pArguments->pUrlListPath);
			retVal = -3;
			return retVal;
		}
	}
	else
	{
		// we need not open urlList second.
		printf("we need not open urlList second!\n");
	}

	retVal = getUrlList(fpUrlList, pArrayUList, &urlItmeNum);
	if (retVal == 0)
	{
		//printf("fseek success!\n");
		//fseek(fpUrlList,0L,SEEK_CUR);
		fclose(fpUrlList);
	}
	else
	{
		printf("get url from urlList File failed\n");
		retVal = -4;
		return retVal;
	}

	// mmap and get the registers.
	g_pMmapAddr = VabMmap(NULL, mmapAddrLength, PROT_READ | PROT_WRITE,
			MAP_SHARED, fdDevice, 0);
	// polling the dsp can be written to.
	if (VabTrue((int) g_pMmapAddr))
	{

		pLinkLayerBuffer->pOutBuffer = (uint32_t *) ((uint8_t *) g_pMmapAddr
				+ PAGE_SIZE * 2);
		pLinkLayerBuffer->pInBuffer = (uint32_t *) ((uint8_t *) g_pMmapAddr
				+ PAGE_SIZE * 2 * 2);

		VabGetInfo(g_pMmapAddr, &pUrlNums, &pDownloadPicNums, &pFailedPicNUms);

		waitWriteBufferReadyParam = VabParamAssign(LINKLAYER_IO_WRITE, WAITTIME,
				&status);
		// dsp should init the RD register to empty in DSP.
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_WAITBUFFERREADY,
				&waitWriteBufferReadyParam);

	}
	else
	{
		printf("mmap failed\n");
		retVal = -5;
		return retVal;
	}

	// write the url to output zone.write the urlNum to output zone.
	if (VabTrue(retIoVal))
	{
		if (status == 0)
		{
			memcpy(pUrlNums, &urlItmeNum, sizeof(int));
			memcpy(pLinkLayerBuffer->pOutBuffer, arrayUrlList,
					(urlItmeNum * URL_ITEM_SIZE));
			printf("loading list to dpu0 ...\n");
		}
		else
		{
			printf("wait writeBuffer timeout\n");
			retVal = -6;
			return retVal;
		}
	}
	else
	{
		printf("ioctl for waitWriteBuffer ready failed\n");
		//timeout,so we wait for the interrupt from dsp to pc
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_WRITE_TIMEOUT,
				&waitWriteBufferReadyParam);
		if (VabTrue(retIoVal))
		{
			memcpy(pUrlNums, &urlItmeNum, sizeof(int));
			memcpy(pLinkLayerBuffer->pOutBuffer, arrayUrlList,
					(urlItmeNum * URL_ITEM_SIZE));
			printf("when timeout,loading list to dpu0 ...\n");
		}
		else
		{
			printf("wait for the interrupt from dsp to pc error\n");
			retVal = -7;
			return retVal;
		}

	}


	// set the pc can be written.
	int rdConfig = LINKLAYER_IO_READ;
	retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &rdConfig);
	if (VabTrue(retIoVal))
	{

		gettimeofday(&downloadStart, NULL);
		//pc change the wt reg and dsp's rd reg status can be read in dsp
		wtConfig = LINKLAYER_IO_WRITE;
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &wtConfig);
	}
	else
	{
		retVal = -8;
		return retVal;
	}

	//send interrupt from pc to dsp, notify dsp,the pc have writen url finish
	pInterruptPollParams->interruptAndPollDirect = 0;
	retIoVal=ioctl(fdDevice, DPU_IO_CMD_INTERRUPT, pInterruptPollParams);
	//retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_INTERRUPT, pInterruptPollParams);
	printf("we start send interrupt to dsp,and notify dsp one thing that pc have wirten finish\n");

	// TODO should change .pc should not read the picture downloaded by the DSP.
	// polling the RD status register ((DSP read the url over and download picture over(DSP change the wt in dsp)) or not).
	// means the url can be overwrite in the dsp zone.,the pc can write agian.

	if (VabTrue(retIoVal))
	{

		waitWriteBufferReadyParam = VabParamAssign(LINKLAYER_IO_READ, WAITTIME,
				&status);
		// dsp change the wt ctl reg means pc can read.()
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_WAITBUFFERREADY,
				&waitWriteBufferReadyParam);

	}
	else
	{
		printf("ioctl for send interrupt failed\n");
		retVal = -10;
		return retVal;

	}

	if (VabTrue(retIoVal))
	{
		if (status == 0)
		{

			gettimeofday(&downloadEnd, NULL);
			timeElapse = ((downloadEnd.tv_sec - downloadStart.tv_sec) * 1000000
					+ (downloadEnd.tv_usec - downloadStart.tv_usec));
			//printf("DSP download url finished\n");
			/*NOTE: dsp finished the download.wait for pc reading.can add read information code here.
			 NOTE: init some parameters in the dsp-side.
			 */
			// read the download status.
			downloadPicNums = *pDownloadPicNums;
			failLoadPicNums = *pFailedPicNUms;

			// init the urlNums to DSP
			*pUrlNums = 0;

			// pc read the finished.single the DSP.
			int wtConfig = LINKLAYER_IO_READ_FIN;
			retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS,
					&wtConfig);
		}
		else
		{
			printf("wait readBuffer status timeout\n");
			retVal = -10;
			return retVal;
		}
	}
	else
	{
		//timeout,so we wait for the interrupt from dsp to pc
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_READ_TIMEOUT,
				&waitWriteBufferReadyParam);
		if (VabTrue(retIoVal))
		{
			if (status == 0)
			{

				gettimeofday(&downloadEnd, NULL);
				timeElapse = ((downloadEnd.tv_sec - downloadStart.tv_sec)
						* 1000000
						+ (downloadEnd.tv_usec - downloadStart.tv_usec));
				// read the download status.
				downloadPicNums = *pDownloadPicNums;
				failLoadPicNums = *pFailedPicNUms;

				// init the urlNums to DSP
				*pUrlNums = 0;

				// pc read the finished.single the DSP.
				int wtConfig = LINKLAYER_IO_READ_FIN;
				retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS,
						&wtConfig);
			}
			else
			{
				printf("wait readBuffer status timeout\n");
				retVal = -11;
				return retVal;
			}
		}
		else
		{
			printf("ioctl DPU_IO_CMD_READ_TIMEOUT failed\n");
			retVal = -12;
			return retVal;
		}

	}
	printf("loading list to dpu0 ...\n");
	printf("done (%d loaded, %d failed, %f ms elapsed).\n", downloadPicNums,
			failLoadPicNums, (timeElapse / 1000));
	if (VabTrue(retIoVal))
	{
		//send interrupt to dsp,and notify dsp one thing that pc have read finish.
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_INTERRUPT, NULL);
		if (VabTrue(retIoVal))
		{
			printf("notify dsp success ...\n");
		}
		else
		{
			printf("notify dsp failed ...\n");
		}
	}
	// init for the next instance.
	failLoadPicNums = 0;
	downloadPicNums = 0;

	// one com finished .init the register.
	*pUrlNums = 0;
	wtConfig = LINKLAYER_IO_WRITE_FIN;
	retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &wtConfig);

	// release the resource.
	VabMunmap(g_pMmapAddr, mmapAddrLength);
	free(pLinkLayerBuffer);
	VabClose(fdDevice);
	return (retVal);
}
int getUrlList(FILE *fpUrlList, char *pUrlList, int *pUrlItmeNum)
{
	int retVal = 0;
	int i = 0;
	char urlItem[256];
	char *pUrlItem = urlItem;
	char *pArrayUrlList = pUrlList;
	int enterCharPos = 0;
	int urlItemNum = 0;
	//while (fgets(pUrlItem, 256, fpUrlList) != NULL)
	while (i < URLNUM)
	{
		fgets(pUrlItem, 256, fpUrlList);
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
		i++;
	}
	i=0;
	*pUrlItmeNum = urlItemNum;
	printf("the url item Num is %d\n", *pUrlItmeNum);
	return (retVal);
}
