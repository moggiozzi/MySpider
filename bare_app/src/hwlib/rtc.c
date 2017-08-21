#include <alt_i2c.h>

static int bcd2bin(unsigned char val) {
	return (val & 15) + ((val) >> 4) * 10;
}
void readRtc() {
	unsigned char buf[8];
	ALT_I2C_DEV_t i2c_dev;
	ALT_STATUS_CODE status = alt_i2c_init(ALT_I2C_I2C0, &i2c_dev);
	status = alt_i2c_op_mode_set(&i2c_dev, ALT_I2C_MODE_MASTER);
	status = alt_i2c_master_target_set(&i2c_dev, 0x50); // 0x50 адрес чипа RTC
	status = alt_i2c_enable(&i2c_dev);

// порядок регистров для PCF8583
//0-control/status
//1-hundredth of a second
//2-seconds
//3-minutes
//4-hours
//5-year/date
//6-weekdays/months
//...
	buf[0] = 1; // индекс первого регистра для чтения
	status = alt_i2c_master_transmit(&i2c_dev, buf, 1, 1, 1);
	status = alt_i2c_master_receive(&i2c_dev, buf, 8, 1, 1);

	struct rtc_time {
		int tm_sec;
		int tm_min;
		int tm_hour;
		int tm_mday;
		int tm_mon;
		int tm_year;
		int tm_wday;
	} dt;

	dt.tm_year = buf[4] >> 6; // счетчик годов 2х битный (0-й год високосный)
	dt.tm_wday = buf[5] >> 5; // день недели 0..6 (0-воскресенье,1-понедельник,...)

	buf[4] &= 0x3f;
	buf[5] &= 0x1f;

	dt.tm_sec = bcd2bin(buf[1]); // секунды 0..50
	dt.tm_min = bcd2bin(buf[2]); // минуты 0..59
	dt.tm_hour = bcd2bin(buf[3]); // часы 0..23
	dt.tm_mday = bcd2bin(buf[4]); // день месяца 1..31
	dt.tm_mon = bcd2bin(buf[5]); // месяц 1..12

	alt_i2c_uninit(&i2c_dev);
}
