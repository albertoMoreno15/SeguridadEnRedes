#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <unistd.h> // Para usar popen
#include <stdbool.h>

#define MAX_PASSWORD_LENGTH 7 // Cambia esto según tus necesidades
#define NUM_CHARACTERS 26 // a-z

// Variable global para indicar si se encontró la contraseña
bool pasw_found = false;

void descifrar_archivo(char *archivo, char *password) {
    // Si ya se encontró la contraseña, no continuar
    if (pasw_found) {
        return;
    }
    printf("Probando contraseña: %s\n", password);
    // Comando GPG para descifrar
    char command[256];
    snprintf(command, sizeof(command), "gpg --batch --quiet --yes --passphrase %s --decrypt %s 2> /dev/null", password, archivo);

    // Ejecutar el comando
    FILE *fp = popen(command, "r");

    // Verificar si se ejecutó exitosamente
    if (fp != NULL) {
        char result[1024];
        // Leer la salida
        if (fgets(result, sizeof(result), fp) != NULL) {
            // Si el resultado es exitoso, se encontró la contraseña
            if (!strstr(result, "Error")) {
                
                
                if (!pasw_found) {
                    printf("La contraseña es: %s\n", password);
                    #pragma omp critical
                    {
                    
                    pasw_found = true; // Actualizar la variable global de manera segura
                    }
                }
                
            }
        }
        pclose(fp);
    }
}

// Generar combinaciones de contraseñas y descifrarlas en paralelo
void generar_combinaciones(char *archivo) {
    char password[MAX_PASSWORD_LENGTH + 1];
    password[MAX_PASSWORD_LENGTH] = '\0'; // Asegura el fin de la cadena
    int len;

    for (len = 4; len <= MAX_PASSWORD_LENGTH; len++) { 
        #pragma omp parallel for private(password) schedule(dynamic)
        for (int i = 0; i < (len > 1 ? NUM_CHARACTERS : 1); i++) {
            for (int j = 0; j < (len > 2 ? NUM_CHARACTERS : 1); j++) {
                for (int k = 0; k < (len > 3 ? NUM_CHARACTERS : 1); k++) {
                    for (int l = 0; l < (len > 4 ? NUM_CHARACTERS : 1); l++) {
                        for (int m = 0; m < (len > 5 ? NUM_CHARACTERS : 1); m++) {
                            for (int n = 0; n < (len > 6 ? NUM_CHARACTERS : 1); n++) {
                                if (!pasw_found) {
                                    password[0] = 'a' + i;
                                    password[1] = len > 2 ? 'a' + j : '\0';
                                    password[2] = len > 3 ? 'a' + k : '\0';
                                    password[3] = len > 4 ? 'a' + l : '\0';
                                    password[4] = len > 5 ? 'a' + m : '\0';
                                    password[5] = len > 6 ? 'a' + n : '\0';
                                    password[len] = '\0'; // Termina la cadena

                                    // Probar la contraseña generada
                                    descifrar_archivo(archivo, password);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <archivo_encriptado>\n", argv[0]);
        return 1;
    }

    char *archivo = argv[1];
    generar_combinaciones(archivo);

    return 0;
}
