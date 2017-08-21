/*
 * system_defs.h
 *
 *  Created on: 15 дек. 2016 г.
 *      Author: AV-Luiya
 */

#ifndef SYSTEM_DEFS_H_
#define SYSTEM_DEFS_H_

#define MAX_COUNT_OF_TABLE (2) // Количество таблиц памяти (1:{таблица переменных}, 2:{таблица буферов})
#define MAX_NUMBER_OF_BUFFERS_IN_TABLE (12) // Количество буферов в таблице

#define MAX_AMOUNT_OF_PROTECTION (64*sizeof(uint32_t)*8) // Максимальное количество защит в проекте


#endif /* SYSTEM_DEFS_H_ */
