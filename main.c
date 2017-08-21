#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <math.h>
#include <tchar.h>
#include <windows.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#define PI 3.14159265

///////////////////////////////////////////////////////////////////////////////
// Configurações
///////////////////////////////////////////////////////////////////////////////
#define N 5                              // Número de filósofos
#define TABLE_RADIUS 0.7f                // Raio da mesa
#define PLATE_RADIUS 0.7f  * 0.9f / N    // Raio do prato
#define CIRCLE_ERROR 5                   // Divisor de precisão dos círculos

///////////////////////////////////////////////////////////////////////////////
// Tamanho dos lados da janela
///////////////////////////////////////////////////////////////////////////////
#define WINDOW_SIZE 640

///////////////////////////////////////////////////////////////////////////////
// Enumerador de estados possíveis para um filósofo
///////////////////////////////////////////////////////////////////////////////
typedef enum __state {
    COMENDO,
    BLOQUEADO,
    ESPERANDO
} ESTADO_FILOSOFO;

///////////////////////////////////////////////////////////////////////////////
// Vetor de filósofos e threads
///////////////////////////////////////////////////////////////////////////////
ESTADO_FILOSOFO filosofos[N];
DWORD timer_filosofos[N];
HANDLE threads_filosofos[N];

///////////////////////////////////////////////////////////////////////////////
// HINSTANCE do programa para carregamento de recursos da memória
///////////////////////////////////////////////////////////////////////////////
HINSTANCE hInst;

///////////////////////////////////////////////////////////////////////////////
// Procedimento dos filósofos
///////////////////////////////////////////////////////////////////////////////
DWORD WINAPI fi_proc(LPVOID);

///////////////////////////////////////////////////////////////////////////////
// Procedimento de desenho
///////////////////////////////////////////////////////////////////////////////
VOID render(GLFWwindow*);

///////////////////////////////////////////////////////////////////////////////
// Ponto de entrada
///////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    hInst = hThisInstance;

    if (!glfwInit())
    {
        fprintf(stderr, "Falha ao inicializar o GLFW");
        return -1;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(
        WINDOW_SIZE,
        WINDOW_SIZE,
        "Filosofos do Sushi",
        NULL,
        NULL
    );

    glfwMakeContextCurrent(window);

    int i;
    for (i = 0; i < N; i++)
    {
        filosofos[i] = ESPERANDO;
        threads_filosofos[i] = CreateThread(NULL, 0, fi_proc, (LPVOID)i, 0, NULL);
    }

    while (!glfwWindowShouldClose(window))
    {
        render(window);
        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    //WaitForMultipleObjects(N, threads_filosofos, TRUE, INFINITE);

    for (i = 0; i < N; i++)
        TerminateThread(threads_filosofos[i], 0);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Procedimento dos filósofos
///////////////////////////////////////////////////////////////////////////////
DWORD WINAPI fi_proc(LPVOID param)
{
    int n = (int)param;

    Sleep((rand() % 5 + 1) * 1000);

    while (1)
    {
        // Espera enquanto estiver bloqueado
        do
        {
            Sleep((rand() % 5 + 1) * 500);
        } while (filosofos[n] == BLOQUEADO || filosofos[(n + 1) % N] != ESPERANDO);

        // Pega o garfo do filósofo à direita
        filosofos[(n + 1) % N] = BLOQUEADO;

        // Come
        filosofos[n] = COMENDO;
        printf("%d esta comendo\r\n", n + 1);
        printf("%d esta bloqueado\r\n", (n + 1) % N + 1);

        timer_filosofos[n] = (rand() % 5 + 1) * 1000;

        // Espera um tempo
        Sleep(timer_filosofos[n]);

        // Libera o cara da direita
        filosofos[n] = ESPERANDO;
        printf("%d esta esperando\r\n", n + 1);

        filosofos[(n + 1) % N] = ESPERANDO;
        printf("%d esta esperando\r\n", (n + 1) % N + 1);

        // Espera um tempo
        Sleep((rand() % 5 + 1) * 1000);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Desenha um círculo
///////////////////////////////////////////////////////////////////////////////
VOID draw_circle(GLfloat ox, GLfloat oy, GLfloat radius)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(ox, oy, 0);

    glBegin(GL_TRIANGLE_STRIP);

    int i;
    for (i = 0; i < 180; i += CIRCLE_ERROR)
    {
        float theta = i * PI / 180;
        glVertex2f(radius * cos(theta), radius * sin(theta));
        glVertex2f(radius * cos(PI * 2 - theta), radius * sin(PI * 2 - theta));
    }

    glEnd();

    glPopMatrix();
}

///////////////////////////////////////////////////////////////////////////////
// Desenha um hashi
///////////////////////////////////////////////////////////////////////////////
VOID draw_hashi(GLfloat ox, GLfloat oy, GLfloat theta)
{
    glColor3ub(237, 222, 177);

    GLfloat size = PLATE_RADIUS;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(ox, oy, 0);
    glRotatef(theta * 180 / PI, 0, 0, 1);

    glBegin(GL_QUADS);

    glVertex2f(0, 0);
    glVertex2f(size, 0);
    glVertex2f(size, size * 0.05f);
    glVertex2f(0, size * 0.05f);

    glEnd();

    glPopMatrix();
}

///////////////////////////////////////////////////////////////////////////////
// Desenha o prato do enésimo filósofo
///////////////////////////////////////////////////////////////////////////////
VOID draw_plate(int n)
{
    GLfloat theta = 2 * PI / N * n, r = (TABLE_RADIUS - PLATE_RADIUS) * 0.9f;
    GLfloat ox = cos(theta) * r,
            oy = sin(theta) * r;

    ESTADO_FILOSOFO estado = filosofos[n];

    switch (estado)
    {
        case BLOQUEADO:
            glColor3f(0.7f, 0, 0);
            break;

        case COMENDO:
            glColor3f(0, 0.7f, 0);
            break;

        default:
            glColor3f(1, 1, 1);
            break;
    }

    glPushMatrix();
    glTranslatef(ox, oy, 0);

    draw_circle(0, 0, PLATE_RADIUS);

    glColor3f(0.85f, 0.85f, 0.85f);
    draw_circle(0, 0, PLATE_RADIUS / 1.5f);

    if (timer_filosofos[n] > 0)
        timer_filosofos[n]--;

    // Sushi de Salmão
    if (estado == COMENDO && timer_filosofos[n] >= 2100)
    {
        glPushMatrix();
        glTranslatef(PLATE_RADIUS / 6, PLATE_RADIUS / 6, 0);

        glColor3f(0, 0, 0);
        draw_circle(0, 0, PLATE_RADIUS / 5);

        glColor3f(1, 1, 1);
        draw_circle(0, 0, PLATE_RADIUS / 6);

        glColor3f(1, 0.5f, 0.3f);
            glBegin(GL_QUADS);
            glVertex2f(0.5 * PLATE_RADIUS / 5, 0.5 * PLATE_RADIUS / 5);
            glVertex2f(-0.5 * PLATE_RADIUS / 5, 0.5 * PLATE_RADIUS / 5);
            glVertex2f(-0.5 * PLATE_RADIUS / 5, -0.5 * PLATE_RADIUS / 5);
            glVertex2f(0.5 * PLATE_RADIUS / 5, -0.5 * PLATE_RADIUS / 5);
        glEnd();

        glPopMatrix();
    }

    // Kani
    if (estado == COMENDO && timer_filosofos[n] >= 1100)
    {
        glPushMatrix();
        glTranslatef(0, -PLATE_RADIUS / 6, 0);

        glColor3f(0, 0, 0);
        draw_circle(0, 0, PLATE_RADIUS / 5);

        glColor3f(1, 1, 1);
        draw_circle(0, 0, PLATE_RADIUS / 6);

        glColor3f(0.8f, 0.8f, 0.8f);
        glBegin(GL_QUADS);
            glVertex2f(0.5 * PLATE_RADIUS / 5, 0.5 * PLATE_RADIUS / 5);
            glVertex2f(-0.5 * PLATE_RADIUS / 5, 0.5 * PLATE_RADIUS / 5);
            glVertex2f(-0.5 * PLATE_RADIUS / 5, -0.5 * PLATE_RADIUS / 5);
            glVertex2f(0.5 * PLATE_RADIUS / 5, -0.5 * PLATE_RADIUS / 5);
        glEnd();

        glColor3f(1, 0.7f, 0.7f);
        glBegin(GL_QUADS);
            glVertex2f(0.5 * PLATE_RADIUS / 5, 0.5 * PLATE_RADIUS / 5);
            glVertex2f(0.25 * PLATE_RADIUS / 5, 0.5 * PLATE_RADIUS / 5);
            glVertex2f(0.25 * PLATE_RADIUS / 5, -0.5 * PLATE_RADIUS / 5);
            glVertex2f(0.5 * PLATE_RADIUS / 5, -0.5 * PLATE_RADIUS / 5);
        glEnd();

        glPopMatrix();
    }

    // Kappamaki
    if (estado == COMENDO && timer_filosofos[n] >= 100)
    {
        glPushMatrix();
        glTranslatef(-PLATE_RADIUS / 6, PLATE_RADIUS / 6, 0);

        glColor3f(0, 0, 0);
        draw_circle(0, 0, PLATE_RADIUS / 5);

        glColor3f(1, 1, 1);
        draw_circle(0, 0, PLATE_RADIUS / 6);

        glColor3f(0.3f, 0.7f, 0.3f);
        glBegin(GL_QUADS);
            glVertex2f(0.5 * PLATE_RADIUS / 5, 0.5 * PLATE_RADIUS / 5);
            glVertex2f(-0.5 * PLATE_RADIUS / 5, 0.5 * PLATE_RADIUS / 5);
            glVertex2f(-0.5 * PLATE_RADIUS / 5, -0.5 * PLATE_RADIUS / 5);
            glVertex2f(0.5 * PLATE_RADIUS / 5, -0.5 * PLATE_RADIUS / 5);
        glEnd();

        glColor3f(0.6f, 1, 0.6f);
        glBegin(GL_QUADS);
            glVertex2f(0.5 * PLATE_RADIUS / 5, 0.5 * PLATE_RADIUS / 5);
            glVertex2f(-0.5 * PLATE_RADIUS / 5, 0.5 * PLATE_RADIUS / 5);
            glVertex2f(-0.5 * PLATE_RADIUS / 5, -0.25 * PLATE_RADIUS / 5);
            glVertex2f(0.5 * PLATE_RADIUS / 5, -0.25 * PLATE_RADIUS / 5);
        glEnd();

        glPopMatrix();
    }

    glPopMatrix();
}

///////////////////////////////////////////////////////////////////////////////
// Procedimento de desenho
///////////////////////////////////////////////////////////////////////////////
VOID render(GLFWwindow* window)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3ub(104, 47, 14);
    draw_circle(0.0f, 0.0f, TABLE_RADIUS);

    int i;
    for (i = 0; i < N; i++)
    {
        GLfloat theta = 2 * PI / N * i, r = (TABLE_RADIUS - PLATE_RADIUS) * 0.9f;

        draw_plate(i);

        switch (filosofos[i])
        {
            case BLOQUEADO:
                break;

            case COMENDO:
                draw_hashi(cos(theta + TABLE_RADIUS / PLATE_RADIUS * 2) * r, sin(theta + TABLE_RADIUS / PLATE_RADIUS * 2) * r, theta);

            default:
                draw_hashi(cos(theta - TABLE_RADIUS / PLATE_RADIUS * 2) * r, sin(theta - TABLE_RADIUS / PLATE_RADIUS * 2) * r, theta);
                break;
        }
    }
}
