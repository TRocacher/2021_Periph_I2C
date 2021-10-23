/*
 * Driver_LTC2944.h
 *
 *  Created on: 16 oct. 2021
 *      Author: trocache
 */



/*
 *   DESCRIPTION DU CIRCUIT EN TERME DE PROG REG EN I2C...
 *
 *   Power on Reset (POR)
 *   Au passage de SENSE+ au delà de 3.3V
 *   - la jauge fonctionne
 *   - les mesures de tension, courant, temp sont off
 *   - les registres sont mis dans leur état pas défaut.
 *   - Le registre d'accumlation (jauge) est placé à mi échelle : 0x7FFF
 *
 *   Tous les registres sont des 8 bits.
 *
 *   Les mesures :
 *   - Voltage = tension sur SENSE-
 *   - Current = tension sur le shunt
 *
 *   Les registres intéressants (y en a beaucoup d'autres utlisés pour les alertes
 *   dont de seuils à régler divers et variés
 *   - A Status @0x0 : les alertes essentiellement
 *   - B Control @0x1 : réglage du circuit
 *
 *   - C Accumulated charge MSB @0x02,   Q[mAh] =
 *   - D Accumulated charge LSB @0x03
 *
 *   - I Voltage reg MSB @0x08  , V_SENSE-[V] = 70.8*Result (I-J) / 65535   (16bits res)
 *   - J Voltage reg LSB @0x09
 *
 *   - O Current Reg MSB @0x0E ,I[mA]=(0.064 / Rsense(Ohm)) . (Result (O-P) -32767)/32767 (12 bits, les 4 lsb = 0)
 *   - P Current Reg MSB @0x0F
 *
 *   - U Temp Reg  Reg MSB @0x14,    Temp[K] = 510K*Result / 65536 (11 bits, les 5 lsb = 0)
 *   - V Temp Reg  Reg MSB @0x15
 */


#ifndef DRIVER_LTC2944_H_
#define DRIVER_LTC2944_H_



#endif /* DRIVER_LTC2944_H_ */
