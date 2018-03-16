/*
 * events.c
 *
 * Created: 16.3.2018 15:18:14
 *  Author: fkla
 */

#include "events.h"

void Event_PTT_pushed_up_oN_relay1_oN_FAN(void)
{
	timer1_set_state(DISABLE);
	pom = "sw rel1+turn fan   ";
	actual_state = EVENT0;
	TCNT1 = TREL;
	uart_puts("bylo zmacknuto tlacitko, zapinam tedy rele 1 a vetron\n");
	timer1_set_state(ENABLE);
}

void Event_PTT_pushed_down_oFF_Ucc(void)
{
	timer1_set_state(DISABLE);
	pom = "Switch OFF Ucc    ";
	actual_state = EVENT1;
	TCNT1 = TSEQ;
	uart_puts("bylo pusteno tlacitko, vypinam Ucc\n");
	timer1_set_state(ENABLE);
}

void error(void)
{
	uart_puts("Tlacis ale nezapnu se, mam poruchu\n");
	timer1_set_state(DISABLE);
	if (fault_flag == 1)
	{
		pom = "nastala chyba       ";
		uart_puts("Po stlaceni byla nastava chyba, nemohu se spustit\n");
	}
	else
	{
		pom = "Divny, Vse OK          ";
		uart_puts("To je divny, vse je uz ok\n");
	}
}

void E0_on_off_relay2(void)
{
	timer1_set_state(DISABLE);
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

void E1_on_off_bias(void)
{
	timer1_set_state(DISABLE);
	if (way == 1)
	{
		pom = "Switch ON bias   ";
		uart_puts("EVENT1 zapinam bias\n");
		TCNT1 = TSEQ;
		timer1_set_state(ENABLE);
		actual_state = EVENT2;
	}
	else
	{
		pom = "Switch OFF bias    ";
		uart_puts("EVENT 1  vypinam bias\n");
		TCNT1 = TSEQ;
		timer1_set_state(ENABLE);
		actual_state = EVENT0;
	}
}

void E2_on_Ucc_off_relay1(void)
{
	timer1_set_state(DISABLE);
	if (way == 1)
	{
		uart_puts("EVENT2 zapinam Ucc\n");
		pom = "Switch ON Ucc     ";
	}
	else
	{
		uart_puts("EVENT2 vypinam rele 1\n");
	pom    = "Switch OFF rel 1";
    }
}

void fault_off_all(void)
{
	timer1_set_state(DISABLE);                       // TIMER1 stops
	button_ptt_set_irq(DISABLE);                     // ISR from PTT button disable, block transmit
	if (once == 1)                                   // do this only once - turn all down
	{
		uart_puts("nastal FAULT - FAULT postupne vse vypnu a drz?m delsi dobu, tedy tohle vse vcetne vypinani vseho delam znovu...\n");
		pom = "Switch OFF Ucc     ";
		_delay_ms(TSEQ);
		pom = "Switch OFF bias    ";
		_delay_ms(TSEQ);
		pom = "switch OFF rel 2   ";
		_delay_ms(TREL);
		pom = "switch OFF rel 1   ";
		pom = "rozmrdals to       ";
		// set Timer counter value register to time delay for block transmit
		TCNT1 = TFAULT;
		// number of repeats to achieve aim delay time about 20 seconds
		// if first run, fault_count set to short time delay
		fault_count++;
		actual_state = FAULT;               // go to fault in ISR timer1
		once = !once;                       // stop repeat this if-loop
		timer1_set_state(ENABLE);           // set on timer1
	}
	// if fault flag came from ADC, after previous loop jump here and repeats to FCOUNT
	// FCOUNT is set to keep this block for 20 second
	else if (fault_count < FCOUNT)
	{
		TCNT1 = TFAULT;
		fault_count++;
		actual_state = FAULT;
		uart_puts("FAULT_count:");
		uart_putc(fault_count);
		uart_puts("\n");
		timer1_set_state(ENABLE);
	}
	// when accomplish previous rule, program jump here and set state to check status PA ability
	else
	{
		uart_puts("jak probehne x opakovani FAULTu (v prvnim startu prednastaveny jenom na jedno projet?), pak zkontroluju jestli je uz vse OK SKOKEM na after FAULT\n");
		pom = "Checking process   ";
		actual_state = AFTER_FAULT;
		TCNT1 = 65520;                    // jump quickly to ISR Timer1 into AFTER_FAULT
		// this is test test rule,
		// means that everything is good and routine in ISR_TIMER1 AFTER_FAULT do nothing and unblock PA
		// in next version this test rule will not be used, and fault_flag will be set after AD conversion
		fault_flag = 0;
		// test setings
		// this variable "once" will be set again after ADC
		once = 1;
		timer1_set_state(ENABLE);                // run TIMER1
	}
}

// this function set ADC on and check status of PA ability
void after_fault_check_status(void)
{
	timer1_set_state(DISABLE);
	if (fault_flag)
	{
		uart_puts("Pusteni ADC a cekani na komparaci, nyni simuluji ze vse OK pevnym nastavim fault_flag = 0\n");
		pom = "SW&RD ADC a COMP    ";
		fault_flag = 0;
		actual_state = AFTER_FAULT;
		TCNT1 = 250;
		timer1_set_state(ENABLE);
	}
	else
	{
		uart_puts("pokud vse OK, pak skace sem a nastavi curent state EVENT0, tedy umozneno startovat pres tlacitko do UP, DW pokud komparator nespusti FAULT \n");
		pom = "Vse OK           ";
		button_ptt_set_irq(ENABLE);
		actual_state = EVENT0;
	}
}

void test_state_of_PTT_button(void)
{
	timer1_set_state(DISABLE);
	uart_puts("jsem v case PTT\n");
	uart_putc(PIND & (1<<2));
	actual_state = old_state;
	if (  button_ptt_is_pressed() && actual_state != FAULT && actual_state != AFTER_FAULT )
	{
		uart_puts("zapinam\n");
		way = 1;
		Event_PTT_pushed_up_oN_relay1_oN_FAN();
	}
	else if ( !button_ptt_is_pressed() && actual_state != FAULT && actual_state != AFTER_FAULT )
	{
		uart_puts("vypinam\n");
		way = 0;
		Event_PTT_pushed_down_oFF_Ucc();
	}
	else
	{
		error();
	}
}

void Event_PTT_button_status_changed(void)
{
	timer1_set_state(DISABLE);
	TCNT1 = 64910;
	if (once)
	{
		old_state = actual_state;
		once = !once;
	}
	actual_state = TEST_PTT;
	uart_puts("Preruseni ISR INT0, skace do test_PTT\n");
	timer1_set_state(ENABLE);
}
