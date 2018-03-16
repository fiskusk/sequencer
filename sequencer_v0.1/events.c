#include "events.h"

void E0_on_relay2(void)
{
	if (way == 1)
	{
		pom = "Switch ON rel 2    ";
		uart_puts("EVENT0 zapinam rele 2\n");
		TCNT1 = TSEQ;
		timer1_set_state(ENABLE);
		actual_state = EVENT1;
	}
	else
	{
		pom = "Switch OFF rel 2    ";
		uart_puts("EVENT0 vypinam rele 2\n");
		TCNT1 = TREL;
		actual_state = EVENT2;
		timer1_set_state(ENABLE);
	}
}
