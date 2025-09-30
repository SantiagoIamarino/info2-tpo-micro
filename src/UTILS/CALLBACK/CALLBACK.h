/*
 * CALLBACK.h
 *
 *  Created on: 23 ago. 2023
 *      Author: gusta
 */

#ifndef UTILS_CALLBACK_CALLBACK_H_
#define UTILS_CALLBACK_CALLBACK_H_

class CALLBACK {

public:
	CALLBACK();

	/*Este método debe ser implementado por las clases
	derivadas cada una resolverá que hacer con su Irq
	enganchada al  systick del sistema	*/
	virtual void Callback( void );

	static void SysTickCall(void);
};

#endif /* UTILS_CALLBACK_CALLBACK_H_ */
