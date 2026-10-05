#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Comm parent_comm;
    MPI_Comm_get_parent(&parent_comm);

    if (parent_comm == MPI_COMM_NULL) {
        // --- Бұл MASTER процестер (бастапқы K процесс) ---
        int K = size;
        double* data = NULL;
        if (rank == 0 || rank == 1) {
            data = (double*)malloc(K / 2 * sizeof(double));
            // Массивті толтыру (мысалы)
            for (int i = 0; i < K / 2; i++) data[i] = (rank + 1) * 10.0 + i;
        }

        // 1. Жаңа 2 процесті іске қосу
        MPI_Comm inter_comm;
        MPI_Comm_spawn("ptprj.exe", MPI_ARGV_NULL, 2, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &inter_comm, MPI_ERRCODES_IGNORE);

        // 2. Интеркоммуникаторды бөлу
        // Түс (color) ретінде рангтің жұп/тақтығын қолданамыз
        MPI_Comm split_inter_comm;
        int color = rank % 2; 
        MPI_Comm_split(inter_comm, color, rank, &split_inter_comm);

        // 3. Мәліметті жіберу (Тек 0 және 1 рангтер жібереді)
        if (rank == 0 || rank == 1) {
            // Мұндағы 0 - бұл интеркоммуникатордың екінші тобындағы (Slave) 0-ші процесс
            MPI_Send(data, K / 2, MPI_DOUBLE, 0, 100, split_inter_comm);
            free(data);
        }

        // 4. Жаңа процестен мәлімет алу (Scatter арқылы)
        double received_val;
        MPI_Scatter(NULL, 1, MPI_DOUBLE, &received_val, 1, MPI_DOUBLE, MPI_ROOT, split_inter_comm);
        
        printf("Master Rank %d received: %f\n", rank, received_val);

        MPI_Comm_free(&split_inter_comm);
        MPI_Comm_free(&inter_comm);

    } else {
        // --- Бұл SLAVE процестер (жаңадан құрылған 2 процесс) ---
        int slave_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &slave_rank);
        
        int remote_size;
        MPI_Comm_remote_size(parent_comm, &remote_size); // Бұл K-ға тең
        int K = remote_size;

        // 1. Бөліну (Master-мен бірдей түс қолдану керек)
        MPI_Comm split_inter_comm;
        MPI_Comm_split(parent_comm, slave_rank, slave_rank, &split_inter_comm);

        // 2. Мәліметті қабылдап алу
        int count = K / 2;
        double* buffer = (double*)malloc(count * sizeof(double));
        // Master-дың 0-ші рангінен алу (әр топтың басы)
        MPI_Recv(buffer, count, MPI_DOUBLE, 0, 100, split_inter_comm, MPI_STATUS_IGNORE);

        // Мәліметті көрсету (Show функциясының орнына printf)
        for(int i=0; i<count; i++) printf("Slave %d received: %f\n", slave_rank, buffer[i]);

        // 3. Мәліметті Master тобына тарату (Scatter)
        // Slave мұнда "тамыр" (root) болады, сондықтан MPI_ROOT емес, 0 рангі екенін көрсетеді
        MPI_Scatter(buffer, 1, MPI_DOUBLE, NULL, 1, MPI_DOUBLE, 0, split_inter_comm);

        free(buffer);
        MPI_Comm_free(&split_inter_comm);
    }

    MPI_Finalize();
    return 0;
}