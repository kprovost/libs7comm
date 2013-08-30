#ifndef _S7COMM_H_
#define _S7COMM_H_

#include <stdint.h>
#include <stdbool.h>
#include "err.h"

struct s7comm_dev_t;

struct s7comm_dev_t* s7comm_connect(const char *addr);
void s7comm_disconnect(struct s7comm_dev_t *dev);

err_t s7comm_read_db_bit(struct s7comm_dev_t *dev, int db, int number, bool *value);
err_t s7comm_read_db_byte(struct s7comm_dev_t *dev, int db, int number, uint8_t *value);
err_t s7comm_read_db_word(struct s7comm_dev_t *dev, int db, int number, uint16_t *value);

err_t s7comm_write_db_bit(struct s7comm_dev_t *dev, int db, int number, uint8_t value);
err_t s7comm_write_db_byte(struct s7comm_dev_t *dev, int db, int number, uint8_t value);
err_t s7comm_write_db_word(struct s7comm_dev_t *dev, int db, int number, uint16_t value);

err_t s7comm_read_output(struct s7comm_dev_t *dev, int card, int port, bool *value);

err_t s7comm_read_flag_bit(struct s7comm_dev_t *dev, int db, int number, bool *value);
err_t s7comm_read_flag_byte(struct s7comm_dev_t *dev, int db, int number, uint8_t *value);
err_t s7comm_read_flag_word(struct s7comm_dev_t *dev, int db, int number, uint16_t *value);

#endif
