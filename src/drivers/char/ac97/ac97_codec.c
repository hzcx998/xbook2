
#include"./ac97_codec.h"
//#include<mutex>


 int ac97_is_audio(struct snd_ac97 * ac97)
{
	return (ac97->scaps & AC97_SCAP_AUDIO);
}
  int ac97_is_modem(struct snd_ac97 * ac97)
{
	return (ac97->scaps & AC97_SCAP_MODEM);
}

/*
是否为有效寄存器
*/
static int snd_ac97_valid_reg(struct snd_ac97 *ac97, unsigned short reg)
{
	return 0;
	switch (ac97->id)
	 {
		case 1:
			if(reg>=0 &&reg<0x80)
	  	    	return 0;

		default:
			  return 1;
	}
	return 1;
}

void snd_ac97_write(struct snd_ac97 *ac97, unsigned short reg, unsigned short value)
{

	if (snd_ac97_valid_reg(ac97, reg))
	{
		keprint("ac97_reg write is not valid\n");
		return;
	}

	ac97->bus->ops->write(ac97, reg, value);
}

//寄存器读
unsigned short snd_ac97_read(struct snd_ac97 *ac97, unsigned short reg)
{
	if (snd_ac97_valid_reg(ac97, reg))
	{
		keprint("ac97_reg  read is not valid\n");
		return 0;
	}

	return ac97->bus->ops->read(ac97, reg);
}

 unsigned short snd_ac97_read_cache(struct snd_ac97 *ac97, unsigned short reg)
{
	return ac97->regs[reg];
}

void snd_ac97_write_cache(struct snd_ac97 *ac97, unsigned short reg, unsigned short value)
{
	if (snd_ac97_valid_reg(ac97, reg))
	{
		return;
	}
	ac97->regs[reg] = value;
	ac97->bus->ops->write(ac97, reg, value);

}

/*
寄存器缓冲区与值不同则更新
*/
int snd_ac97_update(struct snd_ac97 *ac97, unsigned short reg, unsigned short value)
{
	int change;
	if (snd_ac97_valid_reg(ac97, reg))
		return -EINVAL;
	change = ac97->regs[reg] != value;
	if (change) {
		ac97->regs[reg] = value;
		ac97->bus->ops->write(ac97, reg, value);
	}
	return change;
}
