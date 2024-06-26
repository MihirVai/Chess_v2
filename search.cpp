#include "search.h"

EvalBar::EvalBar(string f)
{
    pst.init_tables();
    fen.input_FEN(f);
    m.fetch_Moves(fen.board, fen.turn, fen.isEnPassant, fen.epSquare, fen.castle_options());
}

// Make functions for complete evaluation and Game Search Tree here

string EvalBar::playOneMove(string move, vector<vector<char>> brd, bool t, bool wck, bool wcq, bool bck, bool bcq, bool isEnp, string epS, int hfc, int fms)
{
    string curr_position = move.substr(1, 2);
    string next_position = move.substr(move.length() - 2, 2);
    pair<int,int> curr_ij, next_ij;
    if (move[0] != 'o' && move[0] != 'O') curr_ij = sij(curr_position);
    if (move[0] != 'o' && move[0] != 'O') next_ij = sij(next_position);
    if (move == "O-O") // white castling king side
    {
        brd[7][4] = 'R';
        brd[7][7] = 'K';
        wck = false;
        wcq = false;
        isEnp = false;
    }
    else if (move == "O-O-O") // black castling queen side
    {
        brd[7][4] = 'R';
        brd[7][0] = 'K';
        wck = false;
        wcq = false;
        isEnp = false;
    }
    else if (move == "o-o") // black castling king side
    {
        brd[0][4] = 'r';
        brd[0][7] = 'k';
        bck = false;
        bcq = false;
        isEnp = false;
    }
    else if (move == "o-o-o") // black castling queen side
    {
        brd[0][4] = 'r';
        brd[0][0] = 'k';
        bck = false;
        bcq = false;
        isEnp = false;
    }
    else if (move[0] == 'P' || move[0] == 'p')
    {
        if (isEnp && (move[3] == 'y' || move[3] == 'Y'))
        {
            string passed_sq;
            if (move[0] == 'P')
            {
                passed_sq = ijs(sij(epS).first + 1, sij(epS).second);
            }
            else
            {
                passed_sq = ijs(sij(epS).first - 1, sij(epS).second);
            }
            brd[sij(passed_sq).first][sij(passed_sq).second] = '.';
            brd[curr_ij.first][curr_ij.second] = '.';
            brd[next_ij.first][next_ij.second] = move[0];
            isEnp = false;
        }
        else
        {
            brd[curr_ij.first][curr_ij.second] = '.';
            brd[next_ij.first][next_ij.second] = move[0];
            isEnp = false;
            if (move[3] == 'Z')
            {
                if ((next_ij.second - 1 >= 0 && brd[next_ij.first][next_ij.second - 1] == 'p') ||
                    (next_ij.second + 1 < 8 && brd[next_ij.first][next_ij.second + 1] == 'p'))
                {
                    isEnp = true;
                    epS = ijs(next_ij.first + 1, next_ij.second);
                }
            }
            else if(move[3]=='z')
            {
                if ((next_ij.second - 1 >= 0 && brd[next_ij.first][next_ij.second - 1] == 'P') ||
                    (next_ij.second + 1 < 8 && brd[next_ij.first][next_ij.second + 1] == 'P'))
                {
                    isEnp = true;
                    epS = ijs(next_ij.first - 1, next_ij.second);
                }
            }
        }
    }
    else
    {
        brd[curr_ij.first][curr_ij.second] = '.';
        brd[next_ij.first][next_ij.second] = move[0];
        isEnp = false;
    }
    t = !t;
    // display_board(brd); // for testing purposes
    return fen.get_FEN(brd, t, wck, wcq, bck, bcq, isEnp, epS, hfc, fms);
}

double EvalBar::complete_eval(EvalParams pr)
{
    double eval = evaluate_material(pr.board);
     eval += evaluate_pawn_structure(reverseBoard(pr.board), pr.controlSquares, pr.oppControlSquares, pr.turn, pr.f);
     eval += evaluate_outposts(pr.board, pr.controlSquares, pr.oppControlSquares, pr.turn);
     eval += hanging_piece_penalty(pr.board, pr.controlSquares, pr.oppControlSquares, pr.turn);
     eval += weaker_attacked_penalty(pr.board, pr.controlSquares, pr.oppControlSquares, pr.turn);
     eval += mobility(pr.board, pr.controlSquares, pr.oppControlSquares, pr.turn);
    eval += pieces_eval(pr.board, pr.pieces, pr.oppPieces, pr.turn);
    eval += (double)pst.eval_sq_tables(pr.board)/625.0;
     double king_score = eval_kingsafety(pr.board, pr.controlSquares, pr.oppControlSquares, pr.turn);
    if(gamePhase > 24)
    {
        // eval += king_score;
    }
    return eval;
}

pair<string, double> EvalBar::evalTree(string f, int d ){
    if(d<=0){
         cout<<"Invalid depth for evaluation\n";
         return {"_____",0.0};
    }
    cout << f << " at depth: " << d << endl;
    Board_FEN temp_fen(f);
    Moves temp_Moves(temp_fen.return_board(),temp_fen.return_turn(),temp_fen.return_ep(),temp_fen.return_eps(),temp_fen.castle_options());
    vector<string> my_moves=temp_Moves.valid_Moves();

    double check_for_end=evaluate_checkmate(temp_fen.return_board(),temp_Moves.return_oppControlSquares(),temp_Moves.valid_Moves(),temp_fen.return_turn(),f);
   
    if(check_for_end==inf||check_for_end==-inf||(check_for_end==0.0 && my_moves.size()==0)){
        return {"_",check_for_end};
    }
    
    if(d==1){
            
            int cas_opt=temp_fen.castle_options();
            pair<string,double> result={"_",0.0};
            for(auto move: my_moves){
                 cout << move << ' ';
                 string res=playOneMove(move,temp_fen.return_board(),temp_fen.return_turn(),((cas_opt&8)!=0),((cas_opt&4)!=0),((cas_opt&2)!=0),((cas_opt&1)!=0),temp_fen.return_ep(),temp_fen.return_eps(),temp_fen.return_halfmoveclk(),temp_fen.return_fullmoves());
                 cout << res << endl;
                 Board_FEN final_fen(res);

       //          if(d == 1)
       //          {
       //             return {"_", 0.0};
       //          }
                 Moves final_Moves(final_fen.return_board(),final_fen.return_turn(),final_fen.return_ep(),final_fen.return_eps(),final_fen.castle_options());
                 pair<string,double> temp;
                 temp.first=move;
                 
                 EvalParams lmao_mujhe_ni_pata_kya_hai_ye(final_Moves,final_fen,f);
                 temp.second=complete_eval(lmao_mujhe_ni_pata_kya_hai_ye);

                 if(result.first=="_")result=temp;
                 else if(temp_fen.return_turn())if(result.second>temp.second)result=temp;
                 else if(result.second<temp.second)result=temp;
            }

            return result;
    }


    int cas_opt=temp_fen.castle_options();

    pair<string,double> result={"_",0.0};

    for(auto move: my_moves){
                 string res=playOneMove(move,temp_fen.return_board(),temp_fen.return_turn(),((cas_opt&8)!=0),((cas_opt&4)!=0),((cas_opt&2)!=0),((cas_opt&1)!=0),temp_fen.return_ep(),temp_fen.return_eps(),temp_fen.return_halfmoveclk(),temp_fen.return_fullmoves());
              
                 pair<string,double> temp=evalTree(res,d-1);

                 if(result.first=="_"){
                    result=temp;
                    result.first=move;
                 }
                 else if(temp_fen.return_turn())if(result.second>temp.second){
                    result=temp;
                    result.first=move;
                 }
                 else if(result.second<temp.second){
                    result=temp;
                    result.first=move;
                 }
    }
    return result;
}
    