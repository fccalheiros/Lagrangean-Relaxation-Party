#include "main.h"
#include "Configuration.h"


bool perturbacao;
bool linear;

long int tempoInicio;
clock_t  tempoInicioCpu;

void StartStats() {
    tempoInicio = TempoAtual();
    tempoInicioCpu = TempoCpuAtual();
    cout << "------------------------------------------------------------------------------- " << endl;
    cout << "Branch & Bound starting stats" << endl;
    cout << "   Starting time: " << tempoInicio << endl;
    cout << "   CPU starting time: " << tempoInicioCpu << endl;
    cout << "   CLOCKS_PER_SEC: " << CLOCKS_PER_SEC << endl;
    cout << "------------------------------------------------------------------------------- " << endl;
}
void EndStats() {

    cout << endl;
    cout << "------------------------------------------------------------------------------- " << endl;
    cout << "Branch & Bound finish stats" << endl;
    cout << "   Execution time: " << TempoPassado(tempoInicio) << endl;
    cout << "   CPU time: " << TempoCpuPassado(tempoInicioCpu) << endl;
    cout << "   Current CPU time: " << TempoCpuAtual() << endl;
    cout << "------------------------------------------------------------------------------- " << endl;
 
}

// 
// 
//

int main2 (int argc, char * argv[]) {

    srand(13);
    Configuration * config = new Configuration();
    config->PARSE("C:\\Users\\fccal\\Documents\\Pessoal\\mestrado\\instancias\\Configuration.json");
    cout << config->Print();

    RGPLagrangeanRelaxation* algorithm = new RGPLagrangeanRelaxation(config);
    RGPManager *manager = new RGPManager(config, algorithm);
    SearchAlgorithm sa = SearchAlgorithm::BFS;
    if (config->getValue("BRANCHSTRATEGY").compare("DFS") == 0)
        sa = SearchAlgorithm::DFS;

    algorithm->setLagrangeanManager(manager);
    manager->GenerateProblem(argv[1]);
 
    BBTree bbTree(manager, algorithm, sa, config);

    if ( argc == 2 ) { 

        StartStats();
        bbTree.GO();

        string filename = manager->DefaultFilePrefix() + "." + (sa == SearchAlgorithm::BFS ? "BFS" : "DFS") + ".txt";
        bbTree.Print(filename);

        EndStats();
        cout << std::flush;
        return 0;
    }

    FILE *saida = stdout;
    if (argc == 4) {
        saida = fopen(argv[3],"w");
    }

    if ( strcmp(argv[2],"c") == 0 ) {
        cortes(manager,saida);
        return 0;
    }

    perturbacao =  strcmp(argv[2],"p") == 0;
    linear      =  strcmp(argv[2],"l") == 0;

    if ( linear ) {
        cout << manager->PrintLP();
        return 0;
    }
    else if ( perturbacao ) {
        fprintf(saida,"minimize\n");
        fprintf(saida,"%3.12f",Custo(0));
        //fprintf(saida," x%d",ger3._variables[0]->retNome());
        //for (i=1;i< ger3._variables.size();i++) {
        //  if ( (i % 8) == 0 ) fprintf(saida,"\n"); 
        //  fprintf(saida," + %3.12f",Custo(i));
            // fprintf(saida," x%d",ger3._variables[i]->retNome());
        //}
    }

    fprintf(saida, "\nsubject to\n");
    //ger3.para_cada_restricao( Imprime <Constraint *> (saida) );
    //ger3.para_cada_restricaoND( Imprime <Constraint *> (saida) );  


    fprintf(saida,"binary\n");
    //for (i=1;i< ger3._variables.size();i++)  
    //  fprintf(saida,"x%d\n",ger3._variables[i]->retNome());
    fprintf(saida,"end");
    fflush(saida);
    //fclose(saida);
  
    return 0;

    /*** Imprime programa no formato do Beasley *******/

    //cout << ger3._constraints.size() << " " << ger3._variables.size();
    //for (i=0;i< ger3._variables.size();i++) {
    //  if ( i%12 == 0 ) cout << "\n";
    //  cout << ger3._variables[i]->retCusto() << " ";
    // }
    // cout << "\n";
  
    //for (i=0; i< ger3._constraints.size(); i++) { 
    //  ( (Constraint *)(ger3._constraints[i]) )->ImprimeBeasley();
    //  cout << "\n";
    //}

  return 0;

}


void cortes(LagrangeanManager* prob, FILE* saida) {
    unsigned int i, j;
    int num = 0;
    vector <int> vars;

    prob->_constraints.erase(prob->_constraints.begin(), prob->_constraints.end());
    cout << "li sol" << endl;
    FILE* fp = fopen("sol", "r");
    std::ignore = fscanf(fp, "%d", &num);
    while (num != -1) {
        vars.push_back(num);
        cout << num << " ";
        //    ((RGPVariable *)prob->_variables[num])->ImprimeRetangulo();
        cout << endl;
        std::ignore = fscanf(fp, "%d", &num);
    }
    fclose(fp);

    Grafo g;
    for (i = 0; i < vars.size(); i++)
        g.InsereNo(prob->_variables[vars[i]]);


    for (i = 0; i < vars.size() - 1; i++) {
        for (j = i + 1; j < vars.size(); j++) {
            if ((prob->_variables[vars[i]])->Intercepta((prob->_variables[vars[j]]))) {
                g.InsereAresta(prob->_variables[vars[i]], prob->_variables[vars[j]]);
            }
        }
    }
    g.Imprime();
    g.Clique(prob, 2);
    g.CicloImpar(prob);
    prob->para_cada_corte(Imprime <Constraint*>(saida));
    cout << prob->_cuts.size() << endl;
}

float Psi(bool perturbacao) {
    if (!perturbacao) return (float)0;
    float res = (((float)rand() / (float)1048756) - (float)1024) / (float)1E+10;
    return (float)res;
}

float Custo(int var) {
    // return (float)gerente->_variables[var]->retCusto() + (float)Psi();
    return  0; /// apagar
}
