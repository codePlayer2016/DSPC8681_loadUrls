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
#define URL_ITEM_SIZE (100)
#define URLNUM (4)

typedef struct _tagArguments
{
	char *pUrlListPath;
	//char *pDevicePath;
	int chipIndex;
} Arguments;

//API encapsulation  Vab means video accelarate board

void showHelp(int retVal);
void showError(int retVal);
int parseArguments(int argc, char **argv, Arguments* pArguments);
int loadUrl(Arguments* pArguments);
int getUrlList(FILE *fpUrlList, char *pArrayUrlList, int *pUrlItmeNum);
uint32_t * VabMmap(void *start, size_t length, int prot, int flags, int fd, off_t offsize);

int main(int argc, char **argv)
{
	// TODO: -f <url list file>
	// TODO: -t <device>
	// TODO: --help
	int retVal = 0;
	Arguments arguments;
#if 1
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
#endif
#if 0
	int fd3 = open("/dev/chipUnit3", O_RDWR);
	if (fd3 < 0)
	{
		debug_printf("error\n");
	}
	else
	{
	}
	int fd1 = open("/dev/chipUnit1", O_RDWR);
	if (fd1 < 0)
	{
		debug_printf("error\n");
	}
	else
	{
	}

	uint32_t *g_pMmapAddr[4] =
	{	NULL};
	uint32_t mmapAddrLength = (4 * 1024 * 1024);

	g_pMmapAddr[3] = VabMmap(NULL, mmapAddrLength, PROT_READ | PROT_WRITE, MAP_SHARED, fd3, 0);
	if (-1 == g_pMmapAddr[3])
	{
		debug_printf("error in the mmp\n");
	}
	else
	{
	}
#endif
#if 0
	g_pMmapAddr[1] = VabMmap(NULL, mmapAddrLength, PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
	if (-1 == g_pMmapAddr[1])
	{
		debug_printf("error in the mmp\n");
	}
	else
	{
	}
#endif
	return (retVal);

}
int VabOpen(const char* dev_name, int flag)
{
	int FdHandle = 0;
	FdHandle = open(dev_name, flag);
	if (FdHandle >= 0)
	{
		debug_printf("pcie device open success\n");
		return FdHandle;
	}
	else
	{
		debug_printf("pcie device open error\n");
		return FdHandle;
	}
}
uint32_t * VabMmap(void *start, size_t length, int prot, int flags, int fd, off_t offsize)
{
	uint32_t * MmapAddr = NULL;
	MmapAddr = (uint32_t *) mmap(start, length, prot, flags, fd, offsize);
	if ((int) MmapAddr == -1)
	{
		debug_printf("mmap error!\n");
		return (MmapAddr);
	}
	return (MmapAddr);
}
int VabIoctl(int handle, int command, void *args)
{
	int retVal = 0;
	retVal = ioctl(handle, command, args);
	if (-1 == retVal)
	{
		debug_printf("ioctl error\n");
		retVal = -1;
		return (retVal);
	}
	return retVal;
}
int VabRead(int handle, void *in_buffer, int nbyte)
{
	int rd = 0;
	rd = read(handle, in_buffer, nbyte);
	if (rd < 0)
	{
		debug_printf("read error\n");
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
		debug_printf("write error\n");
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
		debug_printf("seek error\n");
		return -1;
	}
}
#if 0
unsigned int VabSelect(int fd, fd_set*rd, fd_set*wd, fd_set* ed, struct timeval*timeout)
{
	fd_set rfds, wfds;
	FD_ZERO(&rfds);
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	FD_SET(fd, &wfds);
	select(fd + 1, &rfds, &wfds, NULL, NULL);
	if (FD_ISSET(fd, &rfds))
	{
		debug_printf("poll monitor:can be read\n");
	}
	if (FD_ISSET(fd, &wfds))
	{
		debug_printf("poll monitor:can be written\n");
	}

}
#endif
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
DPUDriver_WaitBufferReadyParam VabParamAssign(int waitType, uint32_t pendTime, int32_t * status)
{
	DPUDriver_WaitBufferReadyParam readyParam;
	readyParam.waitType = waitType;
	readyParam.pendTime = pendTime;
	readyParam.pBufStatus = status;
	return readyParam;
}
void VabGetInfo(uint32_t * mmapAddr, uint32_t **urlNums, uint32_t **downloadPicNums, uint32_t **failedPicNUms)
{
	///PC_urlNumsReg---DSP_urlNumsReg,write to dsp.
	*urlNums = (uint32_t *) ((uint8_t *) mmapAddr + PAGE_SIZE + 3 * sizeof(uint32_t));

	///getPicNumers---getPicNumers ,read from dsp.
	*downloadPicNums = (uint32_t *) ((uint8_t *) mmapAddr + 5 * sizeof(uint32_t));

	///failPicNumers---failPicNumers,read from dsp.
	*failedPicNUms = (uint32_t *) ((uint8_t *) mmapAddr + 6 * sizeof(uint32_t));

}
void VabMunmap(uint32_t *addr, int size)
{
	munmap(addr, size);
}
void VabClose(int handle)
{
	close(handle);
}
void showHelp(int retVal)
{
	debug_printf("--help\n");
	debug_printf("input the command as follow: ");
	debug_printf("load-urls -f <url-list> -t <device>\n");
	debug_printf("-t <device>\n");
	debug_printf("\t specify the target device\n");
	debug_printf("-f <url-list>\n");
	debug_printf("\t input the url-list file\n");
}
void showError(int retVal)
{
}
int parseArguments(int argc, char **argv, Arguments* pArguments)
{
	int retVal = 0;
	//int index = 0;
	if (argc == 5)
	{
		if (strcasecmp("-f", argv[1]) == 0)
		{
			pArguments->pUrlListPath = argv[2];
			debug_printf("the input file is %s\n", pArguments->pUrlListPath);
		}
		else
		{
			retVal = -1;
			debug_printf("error:input file is %s\n", argv[2]);
		}

		if ((retVal == 0) && (pArguments->pUrlListPath != NULL))
		{
			if (strcasecmp("-t", argv[3]) == 0)
			{
				//pArguments->pDevicePath = argv[4];
				pArguments->chipIndex = atoi(argv[4]);
				debug_printf("chipIndex=%d\n", pArguments->chipIndex);
			}
			else
			{
				retVal = -2;
				debug_printf("error:chipIndex is %d\n", atoi(argv[4]));
			}
		}

	}
	else
	{
		debug_printf("error:the para number is %d\n", argc);
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

	debug_printf("the urlList file is %s\n", pArguments->pUrlListPath);
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
	FILE *fpUrlList = NULL;
	const char dev_name[] = "/dev/chipUnit0";
	char arrayUrlList[50 * 100];
	char *pArrayUList = arrayUrlList;
	int urlItemNum = 0;
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
	interruptAndPollParam *pInterruptPollParams = (interruptAndPollParam *) &interruptPollParams;

	LinkLayerBuffer *pLinkLayerBuffer = (LinkLayerBuffer *) malloc(sizeof(LinkLayerBuffer));

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

	// get the url from the urlList.txt.
	fpUrlList = fopen(pArguments->pUrlListPath, "rb");
	if (fpUrlList != NULL)
	{
		debug_printf("open file success!\n");
	}
	else
	{
		debug_printf("open the %s failed\n", pArguments->pUrlListPath);
		retVal = -3;
		return retVal;
	}

	retVal = getUrlList(fpUrlList, pArrayUList, &urlItemNum);
	if (retVal == 0)
	{
		debug_printf("urlItemNum=%d\n", urlItemNum);
		fclose(fpUrlList);
	}
	else
	{
		debug_printf("get url from urlList File failed\n");
		retVal = -4;
		return retVal;
	}
	// mmap and get the registers.
	g_pMmapAddr = VabMmap(NULL, mmapAddrLength, PROT_READ | PROT_WRITE, MAP_SHARED, fdDevice, 0);

	pLinkLayerBuffer->pOutBuffer = (uint32_t *) ((uint8_t *) g_pMmapAddr + PAGE_SIZE * 2);
	pLinkLayerBuffer->pInBuffer = (uint32_t *) ((uint8_t *) g_pMmapAddr + PAGE_SIZE * 2 * 2);

	// TODO this is error:
	//VabGetInfo(g_pMmapAddr, &pUrlNums, &pDownloadPicNums, &pFailedPicNUms);
	pUrlNums = (uint32_t *) ((uint8_t *) g_pMmapAddr + PAGE_SIZE + 3 * sizeof(uint32_t));
	///getPicNumers---getPicNumers ,read from dsp.
	pDownloadPicNums = (uint32_t *) ((uint8_t *) g_pMmapAddr + 5 * sizeof(uint32_t));
	///failPicNumers---failPicNumers,read from dsp.
	pFailedPicNUms = (uint32_t *) ((uint8_t *) g_pMmapAddr + 6 * sizeof(uint32_t));

//		LINKLAYER_IO_WRITE_RESET = 1,
//		LINKLAYER_IO_WRITE_QRESET = 2,
//		LINKLAYER_IO_WRITE_FIN = 3,
//		LINKLAYER_IO_WRITE_QFIN = 4,

	//waitWriteBufferReadyParam = VabParamAssign(LINKLAYER_IO_WRITE, WAITTIME, &status); //error!
	//DPUDriver_WaitBufferReadyParam VabParamAssign(int waitType, uint32_t pendTime, int32_t * status)
	//* 1.set the write reset.

	/****************************************pc write to dsp*********************/
	{
		//PC_WT_RESET
		LINKLAYER_IO_TYPE statusChangeCode = LINKLAYER_IO_WRITE_RESET;
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &statusChangeCode);
		//* 2.wai the dsp read reset.//DSP_RD_RESET
		waitWriteBufferReadyParam.waitType = LINKLAYER_IO_WRITE_QRESET;
		waitWriteBufferReadyParam.pendTime = WAITTIME;
		waitWriteBufferReadyParam.pBufStatus = &status;
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_WAITBUFFERREADY, &waitWriteBufferReadyParam);
		debug_printf("polling status=%d\n", status);

		//* 3.write.
		{
			if (status == 0)
			{
				memcpy(pUrlNums, &urlItemNum, sizeof(int));
				memcpy(pLinkLayerBuffer->pOutBuffer, arrayUrlList, (urlItemNum * URL_ITEM_SIZE));
				debug_printf("dsp's readbuf is empty and pc write to\n");
			}
			else
			{
				debug_printf("wait dsp's readbuf empty timeout\n");
				retVal = -6;
				return retVal;
			}
		}

		//* 4.set the write finish.//PC_WT_FINISH
		statusChangeCode = LINKLAYER_IO_WRITE_FIN;
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &statusChangeCode);
		//* 5.wait the dsp read finish.//DSP_RD_FINISH
		waitWriteBufferReadyParam.waitType = LINKLAYER_IO_WRITE_QFIN;
		waitWriteBufferReadyParam.pendTime = WAITTIME;
		waitWriteBufferReadyParam.pBufStatus = &status;
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_WAITBUFFERREADY, &waitWriteBufferReadyParam);
		if (status == 0)
		{
			debug_printf("pc write and dsp read finished\n");
		}
		else
		{
			debug_printf("wait dsp read timeout\n");
			retVal = -6;
			return retVal;
		}
		//* 6.pc write reset.
		statusChangeCode = LINKLAYER_IO_WRITE_RESET;
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &statusChangeCode);
	}
#if 0
	/****************************************pc read and dsp write*********************/
	{
		//* 1. pc read reset
		LINKLAYER_IO_TYPE statusChangeCode = LINKLAYER_IO_READ_RESET;
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &statusChangeCode);
		//* 2. pc wait dsp write.
		waitWriteBufferReadyParam.waitType = LINKLAYER_IO_READ_QFIN;
		waitWriteBufferReadyParam.pendTime = WAITTIME;
		waitWriteBufferReadyParam.pBufStatus = &status;
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_WAITBUFFERREADY, &waitWriteBufferReadyParam);
		//* 3. pc read.
		{

		}
		//* 4. pc read finish.
		LINKLAYER_IO_TYPE statusChangeCode = LINKLAYER_IO_READ_FIN;
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &statusChangeCode);
		//* 5. pc wait dsp write reset.
		waitWriteBufferReadyParam.waitType = LINKLAYER_IO_READ_QRESET;
		waitWriteBufferReadyParam.pendTime = WAITTIME;
		waitWriteBufferReadyParam.pBufStatus = &status;
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_WAITBUFFERREADY, &waitWriteBufferReadyParam);

		//* 6. pc read reset.
		LINKLAYER_IO_TYPE statusChangeCode = LINKLAYER_IO_READ_RESET;
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &statusChangeCode);

	}
#endif
#if 0
	else
	{
		debug_printf("ioctl for waitWriteBuffer ready failed\n");
		//timeout,so we wait for the interrupt from dsp to pc
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_WRITE_TIMEOUT, &waitWriteBufferReadyParam);
		if (VabTrue(retIoVal))
		{
			memcpy(pUrlNums, &urlItemNum, sizeof(int));
			memcpy(pLinkLayerBuffer->pOutBuffer, arrayUrlList, (urlItemNum * URL_ITEM_SIZE));
			debug_printf("when timeout,loading list to dpu0 ...\n");
		}
		else
		{
			debug_printf("wait for the interrupt from dsp to pc error\n");
			retVal = -7;
			return retVal;
		}

	}
#endif
#if 0
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
	retIoVal = ioctl(fdDevice, DPU_IO_CMD_INTERRUPT, pInterruptPollParams);
	//retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_INTERRUPT, pInterruptPollParams);
	debug_printf("we start send interrupt to dsp,and notify dsp one thing that pc have wirten finish\n");

	// TODO should change .pc should not read the picture downloaded by the DSP.
	// polling the RD status register ((DSP read the url over and download picture over(DSP change the wt in dsp)) or not).
	// means the url can be overwrite in the dsp zone.,the pc can write agian.

	if (VabTrue(retIoVal))
	{

		//waitWriteBufferReadyParam = VabParamAssign(LINKLAYER_IO_READ, WAITTIME, &status);
		//DPUDriver_WaitBufferReadyParam VabParamAssign(int waitType, uint32_t pendTime, int32_t * status)

		waitWriteBufferReadyParam.waitType = LINKLAYER_IO_READ;
		waitWriteBufferReadyParam.pendTime = WAITTIME;
		waitWriteBufferReadyParam.pBufStatus = &status;

		// dsp change the wt ctl reg means pc can read.()
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_WAITBUFFERREADY, &waitWriteBufferReadyParam);

	}
	else
	{
		debug_printf("ioctl for send interrupt failed\n");
		retVal = -10;
		return retVal;

	}

	if (VabTrue(retIoVal))
	{
		if (status == 0)
		{

			gettimeofday(&downloadEnd, NULL);
			timeElapse = ((downloadEnd.tv_sec - downloadStart.tv_sec) * 1000000 + (downloadEnd.tv_usec - downloadStart.tv_usec));

			// read the download status.
			downloadPicNums = *pDownloadPicNums;
			failLoadPicNums = *pFailedPicNUms;

			// init the urlNums to DSP
			memset(pLinkLayerBuffer->pOutBuffer, 0, (urlItemNum * URL_ITEM_SIZE));
			*pUrlNums = 0;

			// pc read the finished.single the DSP.
			int wtConfig = LINKLAYER_IO_READ_FIN;
			retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &wtConfig);
		}
		else
		{
			debug_printf("wait readBuffer status timeout\n");
			retVal = -10;
			return retVal;
		}
	}
	else
	{
		//timeout,so we wait for the interrupt from dsp to pc
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_READ_TIMEOUT, &waitWriteBufferReadyParam);
		if (VabTrue(retIoVal))
		{
			if (status == 0)
			{

				gettimeofday(&downloadEnd, NULL);
				timeElapse = ((downloadEnd.tv_sec - downloadStart.tv_sec) * 1000000 + (downloadEnd.tv_usec - downloadStart.tv_usec));
				// read the download status.
				downloadPicNums = *pDownloadPicNums;
				failLoadPicNums = *pFailedPicNUms;

				// init the urlNums to DSP
				memset(pLinkLayerBuffer->pOutBuffer, 0x0, (urlItemNum * URL_ITEM_SIZE));
				*pUrlNums = 0;

				// pc read the finished.single the DSP.
				int wtConfig = LINKLAYER_IO_READ_FIN;
				retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &wtConfig);
			}
			else
			{
				debug_printf("wait readBuffer status timeout\n");
				retVal = -11;
				return retVal;
			}
		}
		else
		{
			debug_printf("ioctl DPU_IO_CMD_READ_TIMEOUT failed\n");
			retVal = -12;
			return retVal;
		}

	}
	debug_printf("loading list to dpu0 ...\n");
	debug_printf("done (%d loaded, %d failed, %f ms elapsed).\n", downloadPicNums, failLoadPicNums, (timeElapse / 1000));

	if (VabTrue(retIoVal))
	{
		//send interrupt to dsp,and notify dsp one thing that pc have read finish.
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_INTERRUPT, NULL);
		if (VabTrue(retIoVal))
		{
			debug_printf("notify dsp success ...\n");
		}
		else
		{
			debug_printf("notify dsp failed ...\n");
		}
	}
	// init for the next instance.
	failLoadPicNums = 0;
	downloadPicNums = 0;

	// one com finished .init the register.
	*pUrlNums = 0;
	wtConfig = LINKLAYER_IO_WRITE_FIN;
	retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &wtConfig);
#endif
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
	while (fgets(pUrlItem, 256, fpUrlList) != NULL)
	{
		if (strlen(pUrlItem) > 20)
		{
			enterCharPos = 0;
			enterCharPos = (strlen(pUrlItem) - strlen("\n"));
			pUrlItem[enterCharPos] = '\0';
			memcpy(pArrayUrlList, pUrlItem, (enterCharPos + 1));
			debug_printf("the url is %s\n", pArrayUrlList);
			pArrayUrlList += URL_ITEM_SIZE;
			urlItemNum++;
		}
		else
		{
			continue;
		}
	}
	*pUrlItmeNum = urlItemNum;
	debug_printf("the url item Num is %d\n", *pUrlItmeNum);
	return (retVal);
}

#if 0
// mmap and get the registers.
//g_pMmapAddr = VabMmap(NULL, mmapAddrLength, PROT_READ | PROT_WRITE, MAP_SHARED, fdDevice, 0);

//uint32_t MmapAddr = NULL;
g_pMmapAddr = (uint32_t *) mmap(NULL, mmapAddrLength, PROT_READ | PROT_WRITE, MAP_SHARED, fdDevice, 0);
if ((void *) g_pMmapAddr != -1)
{
	debug_printf("mmap success!\n");
}
else
{
	debug_printf("mmap failed\n");
	return (-5);
}

// polling the dsp can be written to.

pLinkLayerBuffer->pOutBuffer = (uint32_t *) ((uint8_t *) g_pMmapAddr + PAGE_SIZE * 2);
pLinkLayerBuffer->pInBuffer = (uint32_t *) ((uint8_t *) g_pMmapAddr + PAGE_SIZE * 2 * 2);

VabGetInfo(g_pMmapAddr, &pUrlNums, &pDownloadPicNums, &pFailedPicNUms);

//waitWriteBufferReadyParam = VabParamAssign(LINKLAYER_IO_WRITE, WAITTIME, &status);
waitWriteBufferReadyParam.pBufStatus = &status;
waitWriteBufferReadyParam.pendTime = WAITTIME;
waitWriteBufferReadyParam.waitType = LINKLAYER_IO_WRITE;
// dsp should init the RD register to empty in DSP.
//retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_WAITBUFFERREADY, &waitWriteBufferReadyParam);

retVal = ioctl(fdDevice, DPU_IO_CMD_WAITBUFFERREADY, &waitWriteBufferReadyParam);
if (-1 == retVal)
{
	debug_printf("ioctl error\n");
	return (retVal);
}

memcpy(pUrlNums, &urlItmeNum, sizeof(int));
memcpy(pLinkLayerBuffer->pOutBuffer, arrayUrlList, (urlItmeNum * URL_ITEM_SIZE));
debug_printf("loading list to dpu0 ...\n");

{
	debug_printf("ioctl for waitWriteBuffer ready failed\n");
	//timeout,so we wait for the interrupt from dsp to pc
	retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_WRITE_TIMEOUT, &waitWriteBufferReadyParam);
	if (VabTrue(retIoVal))
	{
		memcpy(pUrlNums, &urlItmeNum, sizeof(int));
		memcpy(pLinkLayerBuffer->pOutBuffer, arrayUrlList, (urlItmeNum * URL_ITEM_SIZE));
		debug_printf("when timeout,loading list to dpu0 ...\n");
	}
	else
	{
		debug_printf("wait for the interrupt from dsp to pc error\n");
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
retIoVal = ioctl(fdDevice, DPU_IO_CMD_INTERRUPT, pInterruptPollParams);
//retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_INTERRUPT, pInterruptPollParams);
debug_printf("we start send interrupt to dsp,and notify dsp one thing that pc have wirten finish\n");

// TODO should change .pc should not read the picture downloaded by the DSP.
// polling the RD status register ((DSP read the url over and download picture over(DSP change the wt in dsp)) or not).
// means the url can be overwrite in the dsp zone.,the pc can write agian.

if (VabTrue(retIoVal))
{

	waitWriteBufferReadyParam = VabParamAssign(LINKLAYER_IO_READ, WAITTIME, &status);
	// dsp change the wt ctl reg means pc can read.()
	retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_WAITBUFFERREADY, &waitWriteBufferReadyParam);

}
else
{
	debug_printf("ioctl for send interrupt failed\n");
	retVal = -10;
	return retVal;

}

if (VabTrue(retIoVal))
{
	if (status == 0)
	{

		gettimeofday(&downloadEnd, NULL);
		timeElapse = ((downloadEnd.tv_sec - downloadStart.tv_sec) * 1000000 + (downloadEnd.tv_usec - downloadStart.tv_usec));

		// read the download status.
		downloadPicNums = *pDownloadPicNums;
		failLoadPicNums = *pFailedPicNUms;

		// init the urlNums to DSP
		memset(pLinkLayerBuffer->pOutBuffer, 0, (urlItmeNum * URL_ITEM_SIZE));
		*pUrlNums = 0;

		// pc read the finished.single the DSP.
		int wtConfig = LINKLAYER_IO_READ_FIN;
		retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &wtConfig);
	}
	else
	{
		debug_printf("wait readBuffer status timeout\n");
		retVal = -10;
		return retVal;
	}
}
else
{
	//timeout,so we wait for the interrupt from dsp to pc
	retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_READ_TIMEOUT, &waitWriteBufferReadyParam);
	if (VabTrue(retIoVal))
	{
		if (status == 0)
		{

			gettimeofday(&downloadEnd, NULL);
			timeElapse = ((downloadEnd.tv_sec - downloadStart.tv_sec) * 1000000 + (downloadEnd.tv_usec - downloadStart.tv_usec));
			// read the download status.
			downloadPicNums = *pDownloadPicNums;
			failLoadPicNums = *pFailedPicNUms;

			// init the urlNums to DSP
			memset(pLinkLayerBuffer->pOutBuffer, 0x0, (urlItmeNum * URL_ITEM_SIZE));
			*pUrlNums = 0;

			// pc read the finished.single the DSP.
			int wtConfig = LINKLAYER_IO_READ_FIN;
			retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_CHANGEBUFFERSTATUS, &wtConfig);
		}
		else
		{
			debug_printf("wait readBuffer status timeout\n");
			retVal = -11;
			return retVal;
		}
	}
	else
	{
		debug_printf("ioctl DPU_IO_CMD_READ_TIMEOUT failed\n");
		retVal = -12;
		return retVal;
	}

}
debug_printf("loading list to dpu0 ...\n");
debug_printf("done (%d loaded, %d failed, %f ms elapsed).\n", downloadPicNums, failLoadPicNums, (timeElapse / 1000));

if (VabTrue(retIoVal))
{
	//send interrupt to dsp,and notify dsp one thing that pc have read finish.
	retIoVal = VabIoctl(fdDevice, DPU_IO_CMD_INTERRUPT, NULL);
	if (VabTrue(retIoVal))
	{
		debug_printf("notify dsp success ...\n");
	}
	else
	{
		debug_printf("notify dsp failed ...\n");
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
while (fgets(pUrlItem, 256, fpUrlList) != NULL)
{
	if (strlen(pUrlItem) > 20)
	{
		enterCharPos = 0;
		enterCharPos = (strlen(pUrlItem) - strlen("\n"));
		pUrlItem[enterCharPos] = '\0';
		memcpy(pArrayUrlList, pUrlItem, enterCharPos);
		debug_printf("the url is %s\n", pArrayUrlList);
		pArrayUrlList += URL_ITEM_SIZE;
		urlItemNum++;
	}
	else
	{
		continue;
	}
}
*pUrlItmeNum = urlItemNum;
debug_printf("the url item Num is %d\n", *pUrlItmeNum);
return (retVal);
}
#endif
