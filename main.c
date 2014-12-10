#include <msp430.h> 

/*
 * main.c
 */
  void setLED(char led);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	//reset all pin config
	P1DIR = 0x00;
	P1OUT = 0x00;
	P1IE = 0x00;
	P2DIR = 0x00;
	P2SEL2 = 0x00;
	P2SEL = 0x00;

    //Clock setup
    BCSCTL2 = SELM_0 | DIVM_0 | DIVS_0;

    if (CALBC1_1MHZ != 0xFF) {
        /* Follow recommended flow. First, clear all DCOx and MODx bits. Then
         * apply new RSELx values. Finally, apply new DCOx and MODx bit values.
         */
        DCOCTL = 0x00;
        BCSCTL1 = CALBC1_1MHZ;      /* Set DCO to 1MHz */
        DCOCTL = CALDCO_1MHZ;
    }

    BCSCTL1 |= XT2OFF | DIVA_0;

    BCSCTL3 = XT2S_0 | LFXT1S_2 | XCAP_1;

    //set up timer @ 250ms interval
    /*
     * TA0CCTL0, Capture/Compare Control Register 0
     *
     * CM_0 -- No Capture
     * CCIS_0 -- CCIxA
     * ~SCS -- Asynchronous Capture
     * ~SCCI -- Latched capture signal (read)
     * ~CAP -- Compare mode
     * OUTMOD_0 -- PWM output mode: 0 - OUT bit value
     *
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    TA0CCTL0 = CM_0 | CCIS_0 | OUTMOD_0 | CCIE;

    /* TA0CCR0, Timer_A Capture/Compare Register 0 */
    TA0CCR0 = 2999;

    /*
     * TA0CTL, Timer_A3 Control Register
     *
     * TASSEL_1 -- ACLK
     * ID_0 -- Divider - /1
     * MC_1 -- Up Mode
     */
    TA0CTL = TASSEL_1 | ID_0 | MC_1;

    //configure pin for motion interrupt
    P1IE |= BIT7;
    P1IES |= BIT7; //high to low
    P1REN |= BIT7;
    P1OUT |= BIT7; //pull-up
    P1IFG = 0x00;;

    P2IE |= BIT6;
	P2IES |= BIT6; //high to low
	P2REN |= BIT6;
	P2OUT |= BIT6; //pull-up
	P2IFG = 0x00;;

    //enable interrupts
    __bis_SR_register(GIE);

	while(1)
	{
	    LPM3; //ACLK remains active - only one we're using
	}
	
}


void setLED(char led)
{
    //set led pins all in high-z (input)
    P1DIR &= ~(0x7F);
    //set all as low so that we only have to set high pins
    P1OUT &= ~(0x7F);
    switch(led)
    {
        case 0:
            //1.0 in, 1.1 out
            P1OUT |= BIT1;
            P1DIR |= BIT0|BIT1;
            break;
        case 1:
            P1OUT |= BIT2;
            P1DIR |= BIT1|BIT2;
            break;
        case 2:
            P1OUT |= BIT0;
            P1DIR |= BIT0 | BIT1;
            break;
        case 3:
            P1OUT |= BIT1;
            P1DIR |= BIT1|BIT2;
            break;
        case 4:
            P1OUT |= BIT2;
            P1DIR |= BIT2|BIT0;
            break;
        case 5:
            P1OUT |= BIT0;
            P1DIR |= BIT0|BIT2;
            break;
        case 6:
            P1OUT |= BIT4;
            P1DIR |= BIT3|BIT4;
            break;
        case 7:
            P1OUT |= BIT5;
            P1DIR |= BIT4|BIT5;
            break;
        case 8:
            P1OUT |= BIT3;
            P1DIR |= BIT3|BIT4;
            break;
        case 9:
            P1OUT |= BIT4;
            P1DIR |= BIT4|BIT5;
            break;
        case 10:
            P1OUT |= BIT5;
            P1DIR |= BIT3|BIT5;
            break;
        case 11:
            P1OUT |= BIT3;
            P1DIR |= BIT3|BIT5;
            break;
        case 12:
            P1OUT |= BIT6;
            P1DIR |= BIT6;
            break;

    }
}

volatile char i, rounds;

/*
 *  ======== Timer0_A3 Interrupt Service Routine ========
 */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR_HOOK(void)
{
    //static char i, rounds;
	if(rounds <= 4)
	{
		setLED(i);

	}
	i++;

    if(i > 12)
    {
        i = 0;
        rounds++;
        if(rounds > 4)
        {
            //rounds = 0;
            setLED(13);//turns all off
            //LPM4; //drop into low power until something happens
        }
    }

    //once we implement fuller code - enter LPM4 to disable this timer
}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    P1IFG = 0x00;
    rounds = 0;
    //LPM4_EXIT;  //wake CPU which will drop us into lpm3
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    P2IFG = 0x00;
    rounds = 0;
    //LPM4_EXIT;  //wake CPU which will drop us into lpm3
}
