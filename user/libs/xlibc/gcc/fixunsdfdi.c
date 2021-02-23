/* 
Convert a to 64bit unsigned integer
*/
unsigned long
__fixunsdfdi (double a)
{
  /* TODO: this func should in gcc - soft-fp, here just simple convert, maybe error! */
  return (unsigned int) a;
}