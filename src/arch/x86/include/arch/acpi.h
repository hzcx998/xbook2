#ifndef _X86_ACPI_H
#define _X86_ACPI_H

unsigned int *acpi_check_RSDPtr(unsigned int *ptr);
unsigned int *acpi_get_RSDPtr(void);
int acpi_checkHeader(unsigned int *ptr, char *sig);
int acpi_enable(void);
int acpi_init(void);
void acpi_shutdown(void);

#endif /* _X86_ACPI_H */
