#include "tetris.h"

static struct sigaction act, oact;

int main(){
	int exit=0;

	initscr();
	noecho();
	keypad(stdscr, TRUE);

	srand((unsigned int)time(NULL));
	createRankList();

	while(!exit){
		clear();
		switch(menu()){
		case MENU_PLAY: play(); break;
		case MENU_EXIT: exit=1;   break;
		case MENU_RANK: rank(); break;
		case MENU_REC : recommendedPlay(); break;
		default: break;
		}
	}

	listExit();
	//if (!recoplay){free(recRoot);}
	//else { free(MrecRoot);}
	endwin();
	system("clear");
	return 0;
}

void InitTetris(){
	int i,j;

	for(j=0;j<HEIGHT;j++)
		for(i=0;i<WIDTH;i++)
			field[j][i]=0;
	blocknum = VISIBLE_BLOCKS;
	if( VISIBLE_BLOCKS < BLOCK_NUM) {
		blocknum = BLOCK_NUM;}
	for (int i= 0 ; i< blocknum ; i++) {
		nextBlock[i] = rand() %7;
	}

	blockRotate=0;
	blockY=-1;
	blockX=WIDTH/2-2;
	score=0;
	gameOver=0;
	timed_out=0;
	recinit();

	if(recoplay) recdisplay();
	DrawOutline();
	DrawField();
	DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate);
	DrawNextBlock(nextBlock);
	PrintScore(score);
}

void DrawOutline(){
	int i,j;
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	DrawBox(0,0,HEIGHT,WIDTH);

	/* next block을 보여주는 공간의 태두리를 그린다.*/
	move(2,WIDTH+10);
	printw("NEXT BLOCK");
	DrawBox(3,WIDTH+10,4,8);
	DrawBox(9,WIDTH+10,4,8);
	/* score를 보여주는 공간의 태두리를 그린다.*/
	move(15,WIDTH+10);
	printw("SCORE");
	DrawBox(16,WIDTH+10,1,8);
}

int GetCommand(){
	int command;

	command = wgetch(stdscr);
	if(recoplay==1 && command !=QUIT) return KEY_DOWN;
	switch(command){
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case FALL:	/* space key*/
		/*fall block*/
		command = FALL;
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command){
	int ret=1;
	int drawFlag=0;
	switch(command){
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
	
		if((drawFlag = CheckToMove(field,nextBlock[0],(blockRotate+1)%4,blockY,blockX)))
			blockRotate=(blockRotate+1)%4;
		break;
	case KEY_DOWN:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)))
		{	blockY++;
			if( recoplay) blockY--;
		}
		break;
	case KEY_RIGHT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX+1)))
			blockX++;
		break;
	case KEY_LEFT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX-1)))
			blockX--;
		break;
	case FALL :
		while(1) {if(!CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)) break;
		blockY++;}
		break; 
	default:
		break;
	}
	if(drawFlag) DrawChange(field,command,nextBlock[0],blockRotate,blockY,blockX);
	
	return ret;
}

void DrawField(){
	int i,j;
	for(j=0;j<HEIGHT;j++){
		move(j+1,1);
		for(i=0;i<WIDTH;i++){
			if(field[j][i]==1){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(".");
		}
	}
}


void PrintScore(int score){
	move(17,WIDTH+11);
	printw("%8d",score);
}

void DrawNextBlock(int *nextBlock){
	int i, j;
	for( i = 0; i < 4; i++ ){
		move(4+i,WIDTH+13);
		for( j = 0; j < 4; j++ ){
			if( block[nextBlock[1]][0][i][j] == 1 ){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}
	for (i=0 ; i<4 ;i++ ) {
		move(10+i ,WIDTH+13);
		for (j=0 ; j<4 ; j++) {
			if (block[nextBlock[2]][0][i][j]) {
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);}
			else printw(" ");
		}
	}
}

void DrawBlock(int y, int x, int blockID,int blockRotate,char tile){
	int i,j;
	for(i=0;i<4;i++)
		for(j=0;j<4;j++){
			if(block[blockID][blockRotate][i][j]==1 && i+y>=0){
				move(i+y+1,j+x+1);
				attron(A_REVERSE);
				printw("%c",tile);
				attroff(A_REVERSE);
			}
		}

	move(HEIGHT,WIDTH+10);
}

void DrawBox(int y,int x, int height, int width){
	int i,j;
	move(y,x);
	addch(ACS_ULCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_URCORNER);
	for(j=0;j<height;j++){
		move(y+j+1,x);
		addch(ACS_VLINE);
		move(y+j+1,x+width+1);
		addch(ACS_VLINE);
	}
	move(y+j+1,x);
	addch(ACS_LLCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void play(){
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	DrawField();
	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}
		command = GetCommand();
		if(ProcessCommand(command)==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
	 		refresh();
			getch();

			return;
		}
	}while(!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
newRank(score);
}

char menu(){
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

/////////////////////////첫주차 실습에서 구현해야 할 함수/////////////////////////

int CheckToMove(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	int i,j;
	for (i=0; i<4; i++) {
		for (j= 0 ; j<4 ;j++ ) {
	if (block[currentBlock][blockRotate][i][j] ) {
		int Nextx = j+ blockX;
		int Nexty = i + blockY;
		if (f[Nexty][Nextx]) return 0;
		if ( Nextx < 0 ) return 0;
		if (Nexty < 0) return 0;
		if (Nextx >= WIDTH) return 0;
		if (Nexty >= HEIGHT) return 0;
		}
		}
	}
	return 1;
}

void DrawChange(char f[HEIGHT][WIDTH],int command,int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	//1. 이전 블록 정보를 찾는다. ProcessCommand의 switch문을 참조할 것.
	//2. 이전 블록 정보를 지운다. DrawBlock함수 참조할 것.
	//3. 새로운 블록 정보를 그린다.

	int prex, prey, prerotate;

	switch (command) {
		case QUIT :
			break;
		case KEY_UP:
			prerotate = (blockRotate+3)%NUM_OF_ROTATE ;
			prex = blockX;
			prey = blockY;
			break;
		case KEY_DOWN :
			prey = blockY -1;
			prex = blockX;
			prerotate= blockRotate;
			break;
		case KEY_RIGHT :
			prex = blockX -1;
			prey = blockY;
			prerotate = blockRotate;
			break;
		case KEY_LEFT :
			prex = blockX +1;
			prey = blockY;
			prerotate = blockRotate;
			break;
		default :
			break;
		}
	int i,j;
	for (i =0 ; i<4 ;i++ ) {
		for (j=0; j<4 ; j++){
		if(block[currentBlock][prerotate][i][j]){

			if (i+prey>=0) {
				move(i+prey+1,j+prex+1);
				printw(".");	}
			}
		}
	}
 while(1) {
	  if(!CheckToMove(f, currentBlock, prerotate, prey+1, prex)) break;
	   prey++;
	  }

	for (i = 0 ; i<4 ;i++) {
		for (j=0 ;j<4 ;j++) {
		if (block[currentBlock][prerotate][i][j] ) {
		if (i+prey>=0) {
			move(i+prey+1, j+prex+1);
			printw(".");	}
	}}}
//DrawField();
	DrawBlockWithFeatures(blockY, blockX, currentBlock,blockRotate);
}

void BlockDown(int sig){
	// user code
	//강의자료 p26-27의 플로우차트를 참고한다.
	if (recoplay== 1) 
	{
	if (CheckToMove(field,nextBlock[0], recommendR, recommendY,recommendX)){
	blockX= recommendX;
	blockY= recommendY;
	blockRotate = recommendR;
	}
	}

	if (CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)) {
	blockY++;
	DrawChange(field, KEY_DOWN, nextBlock[0],blockRotate, blockY, blockX); 	}
	else {
	if (blockY== -1)  {gameOver = 1;}
	else
	{ score += AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX);
		score +=DeleteLine(field);
		PrintScore(score);

		for (int i=0 ; i< blocknum-1 ;i++) {
		nextBlock[i]= nextBlock[i+1];}
		nextBlock[blocknum-1] = rand()%7;
		blockRotate = 0;
		DrawNextBlock(nextBlock);
		blockY = -1; blockX = WIDTH/2-2;
		//if (!recoplay)free(recRoot);
		//else free(MrecRoot);
		recinit();
	//if (recoplay) DrawRecommend(recommendY,recommendX,nextBlock[0],recommendR);
		DrawField();
	}}
	timed_out =0; 
}

int  AddBlockToField(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	//Block이 추가된 영역의 필드값을 바꾼다.
	int i,j;
	int touched=0;
	for (i=0 ; i<4; i++ ) {
		for (j=0 ; j<4 ;j++ ) {

	if (block[currentBlock][blockRotate][i][j]) {
		if((0<=blockX+j && blockX+j<WIDTH) && (0<= blockY +i && blockY+i < HEIGHT)) {
			f[blockY+i][blockX+j] =1;
			if ((blockY+i==HEIGHT-1) || (f[blockY+i+1][blockX+j] ==1)){touched++;
	}}
	}}}

		return touched*10;
}

int DeleteLine(char f[HEIGHT][WIDTH]){
	// user code
	//1. 필드를 탐색하여, 꽉 찬 구간이 있는지 탐색한다.
	//2. 꽉 찬 구간이 있으면 해당 구간을 지운다. 즉, 해당 구간으로 필드값을 한칸씩 내린다.
	int i,j;
	int rowcount=0;
	for (j= HEIGHT -1; j>=0 ; j-- ) {
		for (i = WIDTH-1 ; i>=0; i-- ) {if (f[j][i]==0) { break;}}
		if (i==-1) {
		for (int k = j; k>=1; k--) {
			for (int l = 0 ; l<WIDTH ;l++){
			f[k][l]= f[k-1][l];}}
			rowcount++;
			j= j+1;

		}
	}

	return rowcount*rowcount*100;
}

///////////////////////////////////////////////////////////////////////////

void DrawShadow(int y, int x, int blockID,int blockRotate){

	while(1) {
	if(!CheckToMove(field, blockID, blockRotate, y, x)) break;
	y++;
	}
	y--;
	DrawBlock(y,x,blockID, blockRotate,'/');
}

void DrawBlockWithFeatures( int y, int x, int blockID, int blockRotate) {
	DrawRecommend(recommendY, recommendX, nextBlock[0], recommendR);
	if (!recoplay)DrawShadow(y,x,blockID,blockRotate);
	DrawBlock(y,x, blockID, blockRotate,' ');
}
void createRankList(){
	// user code
	FILE *fp = fopen("rank.txt","r");
	if (fp == NULL) {
		fp = fopen("rank.txt","w");
		if(fp ==NULL) {return ;}
		fprintf(fp,"%d\n",0);
	}

	int ret;
	Node *newnode;
	Node *startnode=0;
	fscanf(fp,"%d",&Nodecount);
	//printw("%d", Nodecount);
	for (int i= 0 ; i<Nodecount;i++)
	{	newnode = malloc(sizeof(Node));
		ret= fscanf(fp,"%s %d", newnode->name, &newnode->score);
		if(ret == -1) break;
		newnode->link =NULL;
		if (Head ==NULL) {
		Head =newnode;
		startnode = newnode;}
		else {
			Head->link = newnode;
			Head = newnode ;
		}
	}
	Head = startnode;
	fclose(fp);
}

void rank(){
	// user code
	int X=1,Y= Nodecount;
	int mode;
	char tempname[NAMELEN];
	clear();
	Node * current = Head;
	Node * delnode;
	Node * prev;
	int flag =0;
	int delnum;
	printw("1.list ranks from X to Y\n");
	printw("2.list ranks by a specific name\n");
	printw("3.delete a specific rank\n");

	mode = getch();
	//printw("%d", Nodecount);
	if (mode =='1') {
	printw("X: ");
	echo();
	scanw("%d", &X);
	printw("Y: ");
	scanw("%d", &Y);
	noecho();
	if ( Y> Nodecount) { Y= Nodecount;  }
	printw("      name       |      score      \n");
	printw("-----------------------------------\n");
	if (X>Y || Nodecount ==0 || X> Nodecount||X<1)
	{  printw("\nsearch failure : no rank in the list\n");}
	else {
	for (int i=1; i< X; i++)
		{current = current ->link;}
	for (int i =0 ; i<Y-X+1 ; i++)
		{
		printw("%-17s| %d\n",current->name, current->score);
		current = current->link;
		}
	}}

	else if (mode =='2') {

		printw("Input the name : ");
		echo();
		scanw("%s",tempname);
		noecho();
		printw("      name       |      score      \n");
		printw("-----------------------------------\n");
	while ( current !=NULL) {
			if (!strcmp(current->name , tempname)) {
			printw("%-17s| %d\n", current->name , current->score);
		flag = 1;
			}
			current = current ->link;
		}
	if (!flag) printw ("\nsearch failure: no name in the list\n");
	}
	else if (mode =='3'){
	printw("Input the rank: ");
	echo();
	scanw("%d",&delnum);
	noecho();
	if (delnum>Nodecount|| delnum<=0)  { printw("\nsearch failure: the rank not in the list\n"); }
	else
	{
		current = Head;
		if ( delnum ==1) {
		delnode = current;
		current = current -> link;
		Head = current;
		free(delnode);
		}
		else {
		for (int i =1 ; i<delnum ; i++){
			prev = current;
			current = current ->link;

		}
		prev->link = current ->link;
		delnode = current;
		free(delnode);
		}
		printw("result: the rank deleted\n");
		Nodecount--;
		writeRankFile();
	}

	}

	getch();

}

void writeRankFile(){
	// user code
FILE * fp = fopen ("rank.txt","w");
fprintf(fp,"%d\n",Nodecount);
Node * temp = Head;
while(temp!=NULL)
{
	fprintf(fp, "%s %d\n", temp ->name, temp->score);
	temp = temp->link;
}
fclose(fp);
}

void newRank(int score){
	// user code
	char name[NAMELEN];
	clear();
	printw("your name : ");
	echo();
	scanw("%s ",name);
	noecho();
	getch();
	Node * prev = NULL;
	Node * cur = Head;

	Node * temp = malloc(sizeof(Node));
	temp->score= score;
	strcpy(temp->name, name);
	temp ->link = NULL;

	if (cur == NULL ) {
	 Head = temp;
	}
	else {
	while (cur != NULL)
	{
		if (score> cur->score) break;
		prev= cur;
		cur = cur ->link;
	}
	temp->link = cur;
	if (prev!=NULL) prev ->link = temp;}
	if (cur == Head)  {
	Head = temp;
	}
	Nodecount++;
	writeRankFile();
}

void listExit() {
	Node* temp = Head;
	Node* delnode;
	while (temp != NULL) {
		delnode = temp;
		temp = temp->link;
		free(delnode);
	}
}
void DrawRecommend(int y, int x, int blockID,int blockRotate){
	DrawBlock(y,x,blockID,blockRotate,'R');
}

void recinit()
{
	if (!recoplay) {
	RecNode RecRoot;
	RecRoot.lv=0;
	RecRoot.score=0;
	for (int j= 0 ;j<HEIGHT;j++){
	for (int i=0 ; i<WIDTH ;i++){
	RecRoot.f[j][i] = field[j][i];}

	}
	recommend(&RecRoot);
		/*recRoot = (RecNode*)malloc(sizeof(RecNode));
		recRoot->lv = 0;
		recRoot->score = 0;
		for (int i = 0; i < CHILDREN_MAX; i++) {
			recRoot->c[i] = NULL;
		}
		for (int j = 0; j < HEIGHT; j++) {
			for (int i = 0; i < WIDTH; i++) {
				recRoot->f[j][i] = field[j][i];
			}
		}
		recommend(recRoot);*/
	}
	else {
		/*MrecRoot = (MRecNode*)malloc(sizeof(MRecNode));
	MrecRoot->lv = 0;
		MrecRoot->score = 0;
		for (int j = 0; j < HEIGHT; j++) {
			for (int i = 0; i < WIDTH; i++) {
				MrecRoot->f[j][i] = field[j][i];
			}
		}
		modified_recommend(MrecRoot);*/

	MRecNode RecRoot ;
	RecRoot.lv=0;
	RecRoot.score=0;
	for (int j= 0 ; j<HEIGHT;j++) {
	for (int i= 0 ; i<WIDTH ;i++ ) {
	RecRoot.f[j][i] = field[j][i];
	}
	}
	modified_recommend(&RecRoot);
	}

}
void reccheck() {
	int i=0;
	do  {
	if (field[1][i] ==1) {gameOver =1; break;}
	i++;
	}while(i<WIDTH);
}
int recommend(RecNode* root) {

	int tempx, tempy;
	int rota = 0, tempscore;
	int final = -1;
	int i, j;
	int idx = 0;
	int finalx, finaly, finalr;
	evalsum+= sizeof(RecNode);
	RecNode temp;
	temp.lv = root->lv + 1;
	for (rota = 0; rota < 4; rota++, idx++) {
		root->c[idx] = &temp;
	//	if (rota >= 2 && nextBlock[root->c[idx]->lv - 1] == 0) continue;
	//	else if (rota >= 2 && nextBlock[root->c[idx]->lv - 1] >= 5) continue;
	//	else if (rota == 1 && nextBlock[root->c[idx]->lv - 1] == 4) continue;
		for (tempx = -2; tempx < WIDTH; tempx++) {
			tempy = 0;
			while (1) {
				if (!CheckToMove(root->f, nextBlock[root->lv], rota, tempy + 1, tempx)) break;
				tempy++;
			}

			if (tempy == -1) continue;
		
			for (j = 0; j < HEIGHT; j++) {
				for (i = 0; i < WIDTH; i++) {
					root->c[idx]->f[j][i] = root->f[j][i];
				}
			}
			root->c[idx]->score = root->score;
			root->c[idx]->score += AddBlockToField(root->c[idx]->f, nextBlock[root->c[idx]->lv - 1], rota, tempy, tempx);
			root->c[idx]->score += DeleteLine(root->c[idx]->f);

			if (root->c[idx]->lv < VISIBLE_BLOCKS) { tempscore = recommend(root->c[idx]); }
			else { tempscore = root->c[idx]->score; }
			if (final <= tempscore)
			{
				final = tempscore;
				finalx = tempx;
				finaly = tempy;
				finalr = rota;
			}
		}
	}
	if (temp.lv - 1 == 0) {
		recommendX = finalx;
		recommendY = finaly;
		recommendR = finalr;
	}
	return final;
}

void recommendedPlay(){
	recoplay =1;
	start= clock();
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM, &act, &oact);
	InitTetris();
	DrawField();
	do {
		if (timed_out == 0) {
			alarm(1);
			timed_out = 1;
		}
		command = GetCommand();
		recdisplay();
		reccheck();
		if (ProcessCommand(command) == QUIT) {
			alarm(0);
			evalsum=0;
			DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
			move(HEIGHT / 2, WIDTH / 2 - 4);
			printw("Good-bye!!");
			refresh();
			getch();
			recoplay=0;
			return;
		}
	}while(!gameOver);

	alarm(0);
	evalsum=0;
	getch();
	DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
	move(HEIGHT / 2, WIDTH / 2 - 4);
	printw("GameOver!!");
	refresh();
	getch();
	recoplay=0;
}


int  modified_recommend(MRecNode* root) {
	int tempx, tempy=0;
	int rota = 0, tempscore;
	int final = -1;
	int i, j;
	int finalx, finaly, finalr;
	MRecNode temp;
	temp.lv = root->lv + 1;
	evalsum+= sizeof(MRecNode);
	
	for (rota = 0; rota < 4; rota++) {
		if (rota >= 2 && nextBlock[temp.lv - 1] == 0) continue;
		else if (rota >= 2 && nextBlock[temp.lv - 1] >= 5) continue;
		else if (rota >= 1 && nextBlock[temp.lv - 1] == 4) continue;

		for (tempx = -2; tempx < WIDTH; tempx++) {
			tempy = -1;
			while (1) {
				if (!CheckToMove(root->f, nextBlock[root->lv], rota, tempy + 1, tempx)) break;
				tempy++;
			}
			
			if (tempy == -1) continue;
			
			for (j = 0; j < HEIGHT; j++) {
				for (i = 0; i < WIDTH; i++) {
					temp.f[j][i] = root->f[j][i];
				}
			}
			temp.score = root->score;
			temp.score += AddBlockToField(temp.f, nextBlock[temp.lv - 1], rota, tempy, tempx);
			temp.score += DeleteLine(temp.f);
			temp.score +=  tempy *tempy ;
			if (temp.lv -1== VISIBLE_BLOCKS && temp.lv>=2) {	
				if (temp.score + 200 <final) continue;
			}
	
			if (temp.lv < VISIBLE_BLOCKS) { 
			tempscore = modified_recommend(&temp); }
			else { tempscore = temp.score; }
			if (final <= tempscore)
			{
				final = tempscore;
				finalx = tempx;
				finaly = tempy;
				finalr = rota;
			}
		}
	}
	if (root->lv == 0) {
		recommendX = finalx;
		recommendY = finaly;
		recommendR = finalr;
	}
	return final;
}
void recdisplay() {

    duration = (double)(clock()-start)/(CLOCKS_PER_SEC);

	move(19, WIDTH + 10);
	printw("TIME(t) ");
	DrawBox(20, WIDTH + 10, 1, 12);
	move(21, WIDTH+16);
	printw("%.4lf",duration);

	move(23, WIDTH + 10);
	printw("SCORE(t)/TIME(t) ");
	DrawBox(24, WIDTH + 10, 1, 12);
	move(25, WIDTH + 15);
	printw("%.4lf", score / duration);

	move(27 ,WIDTH +10);
	printw("SPACE(t) ");
	DrawBox(28,WIDTH+10 ,1,12);
	move(29,WIDTH+15);
	printw("%ld", evalsum);
	
	move(31,WIDTH+12);
	printw("SCORE/SPACE(t) ");
	DrawBox(32,WIDTH+10,1,12);
	move(33,WIDTH+15);
	printw("%.5lf ",(double) score / evalsum);

}
