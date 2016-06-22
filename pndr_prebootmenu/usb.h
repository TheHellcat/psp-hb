/*
 * header for USB functions
 */

void usbEnableUsbMS0(void);
void usbEnableUsbF0(void);
void usbEnableUsbF1(void);
void usbDisableUsb(void);
int usbGetUsbState(void);
int usbGetBytesRead(void);
int usbGetBytesWritten(void);
