/*                                         *\

   ***************************************
   **                                   **
   **    HELLCAT's Demo #1              **
   **    The crappy side of life ;-)    **
   **                                   **
   ***************************************

               == preflight ==

            |\      _,,,---,,_
     ZZZzz /,`.-'`'    -.  ;-;;,_
          |,4-  ) )-,_. ,\ (  `'-'
         '---''(_/--'  `-'\_)

\*                                         */

struct MatrixStream
{
  char          *text;
  unsigned int  color;
  unsigned int  fade;
  unsigned int  x;
  unsigned int  y;
  unsigned int  speed;
  bool          visible;
};

void hcd1InitMatrixStreams(void);
void hcd1UpdateMatrixStreams(void);
void hcd1DrawMatrixStreams(void);
void hcd1FancyWrite1(int x, int y, char* text);
void hcd1RunPart0(void);
