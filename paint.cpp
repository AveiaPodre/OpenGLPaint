#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <forward_list>
#include "glut_text.h"

using namespace std;
#define ESC 27

struct Vertex{
	int x;
	int y;
};

struct Form{
	int type;
	std::forward_list<Vertex> vertexList;
};

std::forward_list<Form> formList;

enum formType{LIN = 1, TRI = 2, RET = 3, POL = 4, CIR = 5};
int mode = LIN;
bool click1 = false;
bool click2 = false;
bool poligon = false;
int i_pol = 0;

int x_m, y_m;
int x_p[99], y_p[99];

int x_tri[3];
int y_tri[3];

int x_origem;
int y_origem;

int width = 1600;
int height = 900;

void pushForm(int type){
    Form f;
    f.type = type;
    formList.push_front(f);
}

void pushVertex(int x, int y){
    Vertex v;
    v.x = x;
    v.y = y;
    formList.front().vertexList.push_front(v);
}

void pushLine(int x1, int y1, int x2, int y2){
    pushForm(LIN);
    pushVertex(x1, y1);
    pushVertex(x2, y2);
}

void pushTri(int x[], int y[]){
	pushForm(TRI);
	pushVertex(x[0], y[0]);
    pushVertex(x[1], y[1]);
    pushVertex(x[2], y[2]);
}

void pushRect(int x1, int y1, int x2, int y2){
	pushForm(RET);
	pushVertex(x1, y1);
	pushVertex(x2, y2);
}

void pushCircle(int x1, int y1, int x2, int y2){
	pushForm(CIR);
	pushVertex(x2, y2);
	pushVertex(x1, y1);
}

void init(void);
void reshape(int w, int h);
void display(void);
void menu_popup(int value);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void mousePassiveMotion(int x, int y);
void drawPixel(int x, int y);
void drawForms();
void Imediato(double x1, double y1, double x2, double y2);
void bresenham(double x1,double y1,double x2,double y2);
void bresenhamCircle(double x1, double y1, double x2, double y2);


int main(int argc, char** argv){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100,100);
	glutCreateWindow("Paint");
	
	init();
	
	glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(mousePassiveMotion);
    
    glutCreateMenu(menu_popup);
    glutAddMenuEntry("Linha", LIN);
    glutAddMenuEntry("Triangulo", TRI);
    glutAddMenuEntry("Retangulo", RET);
	glutAddMenuEntry("Poligono", POL);
	glutAddMenuEntry("Circulo", CIR);
    glutAddMenuEntry("Sair", 0);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	glutMainLoop();
	return EXIT_SUCCESS;
}

void init(void){
    glClearColor(1.0, 1.0, 1.0, 1.0); //Limpa a tela com a cor branca;
}

void reshape(int w, int h){
	// Muda para o modo de projecao e reinicializa o sistema de coordenadas
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Definindo o Viewport para o tamanho da janela
	glViewport(0, 0, w, h);
	
	width = w;
	height = h;
    glOrtho (0, w, 0, h, -1 ,1);  

   // muda para o modo de desenho
	glMatrixMode(GL_MODELVIEW);
 	glLoadIdentity();

}

void display(void){
    glClear(GL_COLOR_BUFFER_BIT); //Limpa o buffer de cores e reinicia a matriz
    glColor3f (0.0, 0.0, 0.0); // Seleciona a cor default como preto
    drawForms(); // Desenha as formas geometricas da lista
    //Desenha texto com as coordenadas da posicao do mouse
    draw_text_stroke(0, 0, "(" + to_string(x_m) + "," + to_string(y_m) + ")", 0.2);
    glutSwapBuffers(); // manda o OpenGl renderizar as primitivas
}

void menu_popup(int form){
	if (form == 0){
		exit(EXIT_SUCCESS);
	}
	mode = form;
}

void keyboard(unsigned char key, int x, int y){
    switch (key){
        case ESC: exit(EXIT_SUCCESS); break;
        case 32: // Código ASCII para a tecla de espaço
			if(poligon){
				poligon = false;
				x_p[0] = 0;
				y_p[0] = 0;
				click1 = false;
				glutPostRedisplay();
			}
            break;
    }
}

void mouse(int button, int state, int x, int y){
    switch (button) {
        case GLUT_LEFT_BUTTON:
            switch(mode){
                case LIN:
                    if(state == GLUT_DOWN){
                        if(click1){
                            x_p[1] = x;
                            y_p[1] = height - y - 1;
                            pushLine(x_p[0], y_p[0], x_p[1], y_p[1]);
                            click1 = false;
                            glutPostRedisplay();
                        }else{
                            click1 = true;
                            x_p[0] = x;
                            y_p[0] = height - y - 1;
                        }
                    }
                	break;
                
                case TRI:
                	if(state == GLUT_DOWN){
                        if(click1){
                        	x_p[1] = x;
                        	y_p[1] = height - y - 1;
                            x_tri[1] = x_p[1];
                            y_tri[1] = y_p[1];
                            click1 = false;
                            click2 = true;
                            glutPostRedisplay();
                        }
                        else if(click2){
							x_tri[2] = x;
							y_tri[2] = height - y - 1;
							pushTri(x_tri, y_tri);
							click2 = false;
							glutPostRedisplay();
						}
						else{
                            click1 = true;
                            x_p[0] = x;
                            y_p[0] = height - y - 1;
                            x_tri[0] = x_p[0];
                            y_tri[0] = y_p[0];
                        }
                    }
                	break;
                
                case RET:
                    if(state == GLUT_DOWN){
                        if(click1){
                            x_p[1] = x;
                            y_p[1] = height - y - 1;
                            pushRect(x_p[0], y_p[0], x_p[1], y_p[1]);
                            click1 = false;
                            glutPostRedisplay();
                        }else{
                            click1 = true;
                            x_p[0] = x;
                            y_p[0] = height - y - 1;
                        }
                    }
                	break;
                
                case POL:
                	if(state == GLUT_DOWN){
						if(click1){
							swap(x_p[0], x_p[1]);
							swap(y_p[0], y_p[1]);
							x_p[1] = x;
							y_p[1] = height - y -1;
							pushVertex(x_p[1], y_p[1]);
							glutPostRedisplay();
						}
						else{
							click1 = true;
							x_p[0] = x_p[1] = x;
							y_p[0] = y_p[1] = height - y -1;
							x_origem = x_p[0];
							y_origem = y_p[0];
							poligon = true;
							pushForm(POL);
							pushVertex(x_p[0], y_p[0]);
						}
					}
					break;
					
                case CIR:
                	if(state == GLUT_DOWN){
						if(click1){
							x_p[1] = x;
							y_p[1] = height - y - 1;
							pushCircle(x_p[0], y_p[0], x_p[1], y_p[1]);
							click1 = false;
							glutPostRedisplay();
						}
						else{
							click1 = true;
							x_p[0] = x;
							y_p[0] = height - y -1;
						}
					}
					break;
            }
 			break;
    }
}

void mousePassiveMotion(int x, int y){
    x_m = x; y_m = height - y - 1;
    glutPostRedisplay();
}

void drawPixel(int x, int y){
	glBegin(GL_POINTS);
		glVertex2i(x, y);
	glEnd();	
}

void drawForms(){
    //Apos o primeiro clique, desenha a reta com a posicao atual do mouse
    if(click1){
    	switch (mode){
			case LIN:
				bresenham(x_p[0], y_p[0], x_m, y_m);
				break;
			
			case TRI:
				bresenham(x_p[0], y_p[0], x_m, y_m);
				break;
			
			case RET:
        		bresenham(x_p[0], y_p[0], x_m, y_p[0]);
                bresenham(x_m, y_p[0], x_m, y_m);
                bresenham(x_m, y_m, x_p[0], y_m);
                bresenham(x_p[0], y_m, x_p[0], y_p[0]);
                break;
            
			case POL:
				bresenham(x_p[0], y_p[0], x_p[1], y_p[1]);
				bresenham(x_p[1], y_p[1], x_m, y_m);
				break;
			
            case CIR:
            	bresenhamCircle(x_p[0], y_p[0], x_m, y_m);
            	break;
		}
	}
	else if(click2){
		switch(mode){
			case TRI:
				bresenham(x_p[0], y_p[0], x_p[1], y_p[1]);
				bresenham(x_p[1], y_p[1], x_m, y_m);
				bresenham(x_m, y_m, x_p[0], y_p[0]);
				break;
		}
	}
    
    //Percorre a lista de formas geometricas para desenhar
    for(forward_list<Form>::iterator f = formList.begin(); f != formList.end(); f++){
    	bool last = f == formList.begin();
    	int i = 0, x[3], y[3];
        switch (f->type){
            case LIN:
                //Percorre a lista de vertices da forma linha para desenhar
                for(forward_list<Vertex>::iterator v = f->vertexList.begin(); v != f->vertexList.end(); v++, i++){
                    x[i] = v->x;
                    y[i] = v->y;
                }
                //Desenha o segmento de reta apos dois cliques
                bresenham(x[0], y[0], x[1], y[1]);
  				break;
  			
			case TRI:
   				for(forward_list<Vertex>::iterator v = f->vertexList.begin(); v != f->vertexList.end(); v++, i++){
					x[i] = v->x;
   					y[i] = v->y;
				}
  				bresenham(x[0], y[0], x[1], y[1]);
        		bresenham(x[1], y[1], x[2], y[2]);
        		bresenham(x[2], y[2], x[0], y[0]);
    			break;
    			
            case RET:
                //Percorre a lista de vertices da forma retangulo para desenhar
                for(forward_list<Vertex>::iterator v = f->vertexList.begin(); v != f->vertexList.end(); v++, i++){
                    x[i] = v->x;
                    y[i] = v->y;
                }
				//Desenha o segmento de reta apos dois cliques
                bresenham(x[0], y[0], x[1], y[0]);
                bresenham(x[1], y[0], x[1], y[1]);
                bresenham(x[1], y[1], x[0], y[1]);
                bresenham(x[0], y[1], x[0], y[0]);
                break;
            
            case CIR:
                //Percorre a lista de vertices da forma retangulo para desenhar
                for(forward_list<Vertex>::iterator v = f->vertexList.begin(); v != f->vertexList.end(); v++, i++){
                    x[i] = v->x;
                    y[i] = v->y;
                }
				//Desenha o segmento de reta apos dois cliques
				bresenhamCircle(x[0], y[0], x[1], y[1]);
                break;

            case POL:
  			  	auto start = f->vertexList.begin();
				auto final = f->vertexList.end();
				auto aux = start;  
				while(aux!= final){
					auto after = next(aux);
					if(after == final){  
						if(poligon && last){
							break;	
						} 
						after  = start;
					}
					bresenham(aux->x,aux->y,after->x,after->y);
					aux++;
				}
				break;            
		}
	}
}

void Imediato(double x1, double y1, double x2, double y2){
    double m, b, yd, xd;
    double xmin, xmax,ymin,ymax;
    
    drawPixel((int)x1,(int)y1);
    if(x2-x1 != 0){ //Evita a divisao por zero
        m = (y2-y1)/(x2-x1);
        b = y1 - (m*x1);

        if(m>=-1 && m <= 1){ // Verifica se o declive da reta tem tg de -1 a 1, se verdadeira calcula incrementando x
            xmin = (x1 < x2)? x1 : x2;
            xmax = (x1 > x2)? x1 : x2;

            for(int x = (int)xmin+1; x < xmax; x++){
                yd = (m*x)+b;
                yd = floor(0.5+yd);
                drawPixel(x,(int)yd);
            }
        }else{ // Se tg menor que -1 ou maior que 1, calcula incrementado os valores de y
            ymin = (y1 < y2)? y1 : y2;
            ymax = (y1 > y2)? y1 : y2;

            for(int y = (int)ymin + 1; y < ymax; y++){
                xd = (y - b)/m;
                xd = floor(0.5+xd);
                drawPixel((int)xd,y);
            }
        }

    }else{ // se x2-x1 == 0, reta perpendicular ao eixo x
        ymin = (y1 < y2)? y1 : y2;
        ymax = (y1 > y2)? y1 : y2;
        for(int y = (int)ymin + 1; y < ymax; y++){
            drawPixel((int)x1,y);
        }
    }
    drawPixel((int)x2,(int)y2);
}

void bresenham(double x1, double y1, double x2, double y2){
	double d, incE, incNE, aux;
	bool S = false, D = false;
	
	double deltaX = (x2 - x1);
	double deltaY = (y2 - y1);
	
	//redução ao primeiro octante
	if((deltaX * deltaY) < 0){
		y1 = -y1;
		y2 = -y2;
		deltaY = (y2 - y1);
		
		S = true;
	}
	if(fabs(deltaX) < fabs(deltaY)){
		aux = x1;
		x1 = y1;
		y1 = aux;
		
		aux = x2;
		x2 = y2;
		y2 = aux;
		
		deltaX = (x2 - x1);
		deltaY = (y2 - y1);
		
		D = true;
	}
	if(x1 > x2){
		aux = x1;
		x1 = x2;
		x2 = aux;
		
		aux = y1;
		y1 = y2;
		y2 = aux;
		
		deltaX = (x2 - x1);
		deltaY = (y2 - y1);
	}
	
	int y = int(y1);
	d = 2*deltaY - deltaX;
	incE = 2*deltaY;
	incNE = 2*deltaY - 2*deltaX;
	
	drawPixel(int(x1), int(y1));
	for(int x = int(x1)+1; x<= int(x2); x++){
		if (d <= 0){
			d = d + incE;
		}
		else{
			d = d + incNE;
			y++;
		}
		
		if(D && S){
			drawPixel(y, -x);
		}
		else if(D){
			drawPixel(y, x);
		}
		else if(S){
			drawPixel(x, -y);
		}
		else{
			drawPixel(x, y);
		}
		
	}
}

void bresenhamCircle(double x1, double y1, double x2, double y2){
	double R = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
	int aux_circ = int(R);
	
	int d = 1 - aux_circ;
	int incE = 3;
	int incSE = -2*aux_circ + 5;
	
	drawPixel(x1, y1 + aux_circ);
	drawPixel(x1, y1 - aux_circ);
	drawPixel(x1 + aux_circ, y1);
	drawPixel(x1 - aux_circ, y1);
	
	int y = aux_circ;
	
	for(int x = 1; y>x; x++){
		if(d<0){
			d = d + incE;
			incE = incE + 2;
			incSE = incSE + 2;
			
			drawPixel(x1 + x, y1 + y);
			drawPixel(x1 + y, y1 + x);
			drawPixel(x1 - x, y1 + y);
			drawPixel(x1 + y, y1 - x);
			drawPixel(x1 + x, y1 - y);
			drawPixel(x1 - y, y1 + x);
			drawPixel(x1 - x, y1 - y);
			drawPixel(x1 - y, y1 - x);
		}
		else{
			d = d + incSE;
			incE = incE + 2;
			incSE = incSE + 4;
			
			drawPixel(x1 + x, y1 + y);
			drawPixel(x1 + y, y1 + x);
			drawPixel(x1 - x, y1 + y);
			drawPixel(x1 + y, y1 - x);
			drawPixel(x1 + x, y1 - y);
			drawPixel(x1 - y, y1 + x);
			drawPixel(x1 - x, y1 - y);
			drawPixel(x1 - y, y1 - x);
			
			y = y - 1;
		}
	}
}
