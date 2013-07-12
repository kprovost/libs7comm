#ifndef _S7COMM_H_
#define _S7COMM_H_

#include <stdint.h>
#include <stdbool.h>
#include "err.h"

struct s7comm_dev_t;

struct s7comm_dev_t* s7comm_connect(const char *addr);
void s7comm_disconnect(struct s7comm_dev_t *dev);

err_t s7comm_read_bit(struct s7comm_dev_t *dev, int db, int number, bool *value);
err_t s7comm_read_byte(struct s7comm_dev_t *dev, int db, int number, uint8_t *value);
err_t s7comm_read_word(struct s7comm_dev_t *dev, int db, int number, uint16_t *value);

err_t s7comm_write_bit(struct s7comm_dev_t *dev, int db, int number, uint8_t value);
err_t s7comm_write_byte(struct s7comm_dev_t *dev, int db, int number, uint8_t value);
err_t s7comm_write_word(struct s7comm_dev_t *dev, int db, int number, uint16_t value);

#endif
