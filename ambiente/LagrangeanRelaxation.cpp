#include <stdio.h>
#include <algorithm>

#include "LagrangeanRelaxation.h"
#include "Constraint.h"
#include "RGPManager.h"
#include "grafo.h"


/************ Comeco da LagrangeanRelaxation *********************/

LagrangeanRelaxation::LagrangeanRelaxation(Configuration *config):
  Algoritmo(config)
{
    _alfa = (float)config->INI_ALFA;
    _lambda = config->LAMBDA;
    _naoMudouLI = 0;
    _ultimoLI = config->MINUS00;
    _ultimoLIHeuristica = _ultimoLI;
    _fracionario = 0;
    _reStart = false;
    _somaMultiplicadores = 0;
    _ordenou = false;
    _valorFracionario = 0;
}


LagrangeanRelaxation::~LagrangeanRelaxation() 
{
}

void LagrangeanRelaxation::setLagrangeanManager(LagrangeanManager *mes) {
    Algoritmo::setLagrangeanManager(mes);
}

void LagrangeanRelaxation::Inicializacao() {
  Algoritmo::Inicializacao();
}



/*************** Relaxacao ******************/



void LagrangeanRelaxation::InicializaRelaxacao(float & soma) {

    ConstraintIterator pComeco, pFim;
    VariableIterator   rComeco, rFim;
    VariableIterator   vComeco, vFim;

  /*
  // Inicializa o custo lagrangeano de cada variavel com seu custo na FO
  for ( _manager->VariableBounds(vComeco, vFim); vComeco != vFim; vComeco++ ) 
     (*vComeco)->iniciaCustoLag();

  soma = 0;  // Soma dos multiplicadores lagrangeanos

  
  // Calcula os novos custo lagrangeanos de cada variavel
  // Percorre todas as restricoes, e para cada uma percorre todas suas variaveis
  for ( _manager->ConstraintsBounds(pComeco, pFim) ; pComeco != pFim ; pComeco++) {
    Constraint *rest = (Constraint *) (*pComeco);
    float multiplicador = rest->getLagrangean();
    if ( ! rest->_fixa ) {
       soma += multiplicador * rest->getRHS();
       for ( rest->ConstraintIterators(rComeco, rFim); rComeco != rFim; rComeco++) {
	  Variable *var = (*rComeco);
	  //float coef = 1.0;
	  //var->_valorLag -= coef*multiplicador;
	  var->_valorLag -= multiplicador;
       }
    }
  }
  */

  
    soma = 0;  // Soma dos multiplicadores lagrangeanos
    for ( _manager->ConstraintsBounds(pComeco, pFim) ; pComeco != pFim ; pComeco++)  
        soma += (*pComeco)->_lagrangean * (*pComeco)->getRHS();

    _manager->VariableBounds(vComeco, vFim);
    for ( ; vComeco != vFim;) {
        Variable *var = (*vComeco);
        var->iniciaCustoLag();
        for (int i = 0; i < var->_linhasCobertas; i++) {
            var->_valorLag -= var->_constraints[i]->_lagrangean;   //Est� assumindo que o coeficiente da restri��o � 1
        }
        vComeco++;
    }


    for ( _manager->CutsBounds(pComeco, pFim) ; pComeco != pFim ; pComeco++) {  
        Constraint *rest = (Constraint *) (*pComeco);
        float multiplicador = rest->getLagrangean();
        soma += multiplicador * rest->getRHS();
        for ( rest->ConstraintIterators(rComeco, rFim); rComeco != rFim; rComeco++) {  
	        ((Variable *)(*rComeco))->_valorLag -= multiplicador;        //Est� assumindo que o coeficiente da restri��o � 1
        }
    }

    _somaMultiplicadores = soma;
}

// generic relaxation without any non dualized constraint
void LagrangeanRelaxation::Relaxacao(Solucao& sol, float& valor, float InitialCost) {

    InicializaRelaxacao(valor);
    valor += InitialCost;
    sol.erase(sol.begin(), sol.end());

    bool naoOrdena = (_iteracoes > 100) && ((_iteracoes % 2) == 0);

    cout << valor << " ** ";

    VariableIterator vIt, vEnd;
    _manager->VariableBounds(vIt, vEnd);

    if (naoOrdena) {
        _ordenou = false;
        while (vIt != vEnd) {
            if ((*vIt)->_valorLag <= 0) {
                sol.push_back(*vIt);
                valor += (*vIt)->_valorLag;
            }
        }
    }
    else {
        _manager->Ordena2(CompareLagrangean <Variable*> ());
        _ordenou = true;
        while ((*vIt)->_valorLag <= 0) {
            sol.push_back(*vIt);
            valor += (*vIt)->_valorLag;
        }
        
    }

    cout << valor << endl << endl;
}



void LagrangeanRelaxation::SubGradiente(Solucao &sol){ 
    int i = 0;
    int var;
    ConstraintIterator rest,fim, restLixo;
    float LI = _manager->getLowerBound();
    float LS = _manager->getUpperBound();
    float tw;
    int solSize = sol.size();

    int tamanho = _manager->_constraints.size() + _manager->_cuts.size();  
    cout << "CUTS: " << _manager->_cuts.size() << endl; 
    vector <float> Gw(tamanho);
    float moduloGw = 0;

    _iteracoes++;

    _manager->ConstraintsBounds(rest,fim);
    for (i=0; rest != fim; rest++ ) {
        Gw[i] = (*rest)->getRHS();
        i++;
    }
    for (var=0 ; var < solSize; var++) {
        for ( i=0; i < sol[var]->_linhasCobertas; i++) 
            Gw[ (sol[var]->_constraints[i])->_index ] -= 1; // Est� cravado em 1 supondo que os indices das vari�veis sempre s�o 1
    }
    _manager->ConstraintsBounds(rest,fim);
    for (i=0; rest != fim; rest++ ) {
        moduloGw += Gw[i] * Gw[i]; 
        i++;
    }

    // Trocar o getCoefficient pelo retintercessao , ver se o corte esta ordenado
    _manager->CutsBounds(rest,fim);
    for (; rest != fim; rest++ ){
        Gw[i] = (*rest)->getRHS();
        for (var=0; var < solSize; var++)
            Gw[i] -= (*rest)->getCoefficient(sol[var]);
        if ( _fracionario != 0 ) 
            Gw[i] -= (*rest)->getCoefficient(_fracionario) * _valorFracionario;
        moduloGw += Gw[i] * Gw[i];
        i++;
    }

   /**** Testes para orientar o passo do subgradiente ****/

    if ( _ultimoLI == LI ) 
        _naoMudouLI++;
    else {
        _naoMudouLI = 0;
        _ultimoLI = LI;
    }
  
    if ( _naoMudouLI == _config->ITERATIONS_CHANGE_ALFA ) {   
        _alfa = _lambda * _alfa;
        _naoMudouLI = 0;
    }

  
    float razao = LI/LS;
    if ( (razao > 0.8) || (razao < -0.8) )
        tw = _alfa*( ( (float)1.02 * LS) - LI );
    else {
        tw = _alfa * LI * (float) 0.2;
        if ( tw < 0 ) tw = -1*tw;
        if (tw == 0) tw = 10;
    }
    /**** Testes para orientar o passo do subgradiente ****/

    i = 0;
    _manager->ConstraintsBounds(rest,fim);
    for (; rest != fim; rest++ ){
        float ml = (*rest)->_lagrangean;
        (*rest)->setLagrangean( ml + tw*Gw[i]/moduloGw );
        i++;
    }  
  
    _manager->CutsBounds(rest,fim);
    bool sai = (fim == rest);
    if ( !sai ) fim--;
    i = tamanho - 1;
    while ( ! sai ) {
        sai = ( fim == rest );
        float ml = (*fim)->_lagrangean;
        (*fim)->setLagrangean( ml + tw*Gw[i]/moduloGw);
        i--;
        if ( (*fim)->_lagrangean == 0 ) {
            restLixo = fim;
            if (!sai) fim --;
            _manager->RemoveCut(restLixo);
        }
        else if ( !sai ) fim--;
    }

}

/********************************************************/
/* Comeco da heuristica para obtencao de solucao viavel */
/********************************************************/

bool LagrangeanRelaxation::TemIntercessao(Solucao &solHeu, Variable *var) {

    for (unsigned int i=0; i < solHeu.size(); i++) {

        if ( var->Intercepta( solHeu[i] ) ) 
            return true;
    }
    return false;

}

void LagrangeanRelaxation::InicializacoesHeuristica() {

    ConstraintIterator rest, restFim;
    _manager->ConstraintsNDBounds(rest, restFim);
    for (; rest != restFim; rest++)
        (*rest)->Uncover();

}

bool LagrangeanRelaxation::Heuristica(Solucao &solRel, Solucao &solHeu, float &valor, float InitialCost) {
  
    //return false;
 
    float LI = _manager->getLowerBound();
    float LS = _manager->getUpperBound();
    _ultimoLIHeuristica = LI;

    Variable *var;
    bool particao = false;
    bool impossivel = false;
    int descobertas = _manager->_countConstraints + _manager->_countConstraintsND; 

    solHeu.erase(solHeu.begin(),solHeu.end());

    InicializacoesHeuristica();
    valor = InitialCost;

    VariableIterator begin, It, end;
    _manager->VariableBounds(It,end);
    while ( ( !particao ) && ( !impossivel ) ) {
        if (valor > LS) return particao;
        var = *It;
        if ( var->Valida() ) {
            if ( ! TemIntercessao(solHeu,var) ) {
	            solHeu.push_back(var);
	            valor += var->retCusto();
	            descobertas -= var->_linhasCobertas;
	        }	
        }

        It++; 
        particao = (descobertas == 0);
        impossivel = (It == end) && ! particao;
    }

    _manager->VariableBounds(begin, end);

    if (particao) {
        cout << "Primal Solution Found: " << valor << " --- " << distance(begin,It) << endl;
    }
    else
        cout << "Primal Solution Fail:  " << descobertas << " Cost: " << valor << " --- " << distance(begin, It) << endl;
  
    return particao;

}

/************ Geracao de cortes ************/

void LagrangeanRelaxation::GeraCortes(Solucao &solRel) {

    if (!_config->CUT_GENERATION) return;

    int i,j;
    int tamanho = (int) solRel.size();
    if (tamanho == 0)
        return;

    Grafo g(tamanho);

    for (i=0; i< tamanho ; i++)
        g.InsereNo(solRel[i]);

    //g.ImprimeNos();
    for (i=0; i < tamanho - 1 ; i++ ) {
        for (j=i+1; j < tamanho ; j++)
            if ( solRel[i]->Intercepta(solRel[j]) ) {
                g.InsereAresta(solRel[i],solRel[j]);
                //cout << "Interceptou: " << solRel[i]->retNome() << " - " << solRel[j]->retNome() << endl;
            }
    }
    //g.Imprime();
    g.Clique(_manager);
    g.CicloImpar(_manager);
}

bool LagrangeanRelaxation::Price(Solucao& relaxed) {

    if (! _manager->OptimalFound()) return false;

    VariableIterator vIt = relaxed.begin();
    VariableIterator vEnd = relaxed.end();
    float max = _config->MINUS00;

    for (; vIt != vEnd; vIt++) {
        if ((*vIt)->_valorLag > max)
            max = (*vIt)->_valorLag;
    }

    _manager->VariableBounds(vIt, vEnd);

    // felipe -- n�o deveria ter refer�ncia a RGPVariable aqui
    int count = 0;
    for (; vIt != vEnd; vIt++) {
        if ((*vIt)->retCustoLag() < max && ((RGPVariable*)(*vIt))->_out) {
            cout << "x" << (*vIt)->retNome() << " : " << (*vIt)->retCustoLag() << endl;
            ((RGPVariable*)(*vIt))->_out = false;
            count++;
        }
    }
    cout << "Pricing Variables Included: " << count << endl;
    return true;
 }


/************* Teste de parada do algoritmo lagrangeano ************/
bool LagrangeanRelaxation::TesteParada() { 

    //if  ( ((_manager->getUpperBound() - _manager->getLowerBound()) < (float) _config->STOP_GAP) ||
     if ( (_manager->OptimalFound()) || (_iteracoes > _config->MAX_ITERATIONS) ) {
        return true; 
    }

    else if (_config->RESTART) {
        if ((_iteracoes % _config->RESTART_ITERATIONS) == 0)
            Restart();
    }

    return false; 
};

void LagrangeanRelaxation::Restart() {
    _alfa = (float)_config->INI_ALFA;
    _lambda = _config->LAMBDA;
    _naoMudouLI = 0;
    _ultimoLI = _config->MINUS00;
    _ultimoLIHeuristica = _ultimoLI;
    _fracionario = 0;   
    _reStart = true;
    _manager->Restart();
}


Variable* LagrangeanRelaxation::ChooseBranchVariableLowLagrangean() {

    VariableIterator vIt, vEnd, vSelected;
    float MIN = _config->PLUS00;

    _manager->VariableBounds(vIt, vEnd);

    for (; vIt != vEnd; vIt++) {
        if ((*vIt)->_valorLag < MIN) {
            MIN = (*vIt)->_valorLag;
            vSelected = vIt;
        }
    }
    return (*vSelected)->CopyAndClean(NULL);

}
Variable* LagrangeanRelaxation::ChooseBranchVariableHighIncumbentCost() {

    VariableIterator vIt, vEnd, vSelected;
    VariableIterator vItAll, vEndAll;

    if (_manager->_best.size() > 0) {

        sort(_manager->_best.begin(), _manager->_best.end(), GreaterCost <Variable*>());

        VariableIterator vIt, vEnd;
        vIt = _manager->_best.begin();
        vEnd = _manager->_best.end();

        for (; vIt != vEnd; vIt++) {
            _manager->VariableBounds(vItAll, vEndAll);
            for (; vItAll != vEndAll; vItAll++) {
                if ((*vIt)->_nome == (*vItAll)->_nome)
                    return (*vIt)->CopyAndClean(NULL);
            }
        }

    }

    return (*(_manager->_variables.begin()))->CopyAndClean(NULL);
}


 Variable* LagrangeanRelaxation::ChooseBranchVariable() {

     if (_config->getValue("VARIABLESTRATEGY").compare("LOWLAGRANGEAN") == 0)
         return ChooseBranchVariableLowLagrangean();

     return ChooseBranchVariableHighIncumbentCost();


 }