/*クオリティCプログラミンング 中級編
**
**例題1:
** 住所録管理表.第1版
**
**file name:denden1.c
**コンパイルする場合、ライブラリのリンクオプション -lncursesw をつけること
**
**composed by Naohiko Takeda
**   ver.1.00. 1990.5.27.
**composed by Yoshihide Chubachi
**   ver.1.10. 2003.4.27.
**composed by Shinsuke Kobayashi
**   ver.2.00. 2019.4.11.
**
*/


/*-------------------------------------------*/
/* ヘッダーファイルの読み込み */
/*-------------------------------------------*/

#include <stdio.h>
#include <ctype.h>
#include <ncurses.h>
#include <locale.h>

/*-----------------------------------------*/
/* システム定数の宣言 */
/*-----------------------------------------*/

#define TRUE  1
#define FALSE  0

#define BUFFER_LEN 80 /*入力バッファの文字数*/

#define RECORD_MAX 20 /* 最大データ件数 */
#define NAME_LEN 40 /* 名前の文字数 */
#define ADDR_LEN 60 /* 住所の文字数 */
#define TEL_LEN 15 /*電話番号の文字数 */
#define ZIP_LEN 9 /*郵便番号の文字数 */

#define TITLE_LINE 1 /* タイトル行の位置 */
#define DATA_LINE 3 /* データ表示行の先頭位置 */
#define DISP_REC_MAX 5 /* 1ページ目に書くデータ件数 */
#define MSG_LINE 18 /* メッセージ行の先頭位置 */

#define UP 1 /* 前ページ */
#define DOWN 0 /* 次ページ */

/* コマンドの定義 */

#define CMD_MAX 4 /* 第1版 */

#define APPEND 'A' /* データの追加 */
#define UP_PAGE 'U' /* 前ページへ */
#define DOWN_PAGE 'D' /*次ページへ */
#define QUIT 'Q' /* 終了 */

char cmd_list[] = {
                    APPEND,
                    UP_PAGE,
                    DOWN_PAGE,
                    QUIT
                  };


/*-----------------------------------*/
/* グローバル変数の宣言 */
/*-----------------------------------*/

int cursor = 0; /* 注目行 */
int rec_qty = 0; /* データの数 */

char j_name[RECORD_MAX][NAME_LEN]; /* 名前（漢字）*/
char k_name[RECORD_MAX][NAME_LEN]; /* 名前（フリガナ）*/
char address[RECORD_MAX][ADDR_LEN]; /* 住所 */
char tel_no[RECORD_MAX][TEL_LEN]; /* 電話番号 */
char zip[RECORD_MAX][ZIP_LEN]; /* 郵便番号 */


/*------------------------------------*/
/* 使用する関数の宣言 */
/*------------------------------------*/

/* コマンド処理用部品 */
char input_cmd(void);
void exec_cmd(char cmd);

/* 住所録管理コマンド */
void data_append(void);
void paging(int direction);

void init_buff();

/* 画面表示用部品 */
void init_disp(void);
void show_one_page(void);
void show_record(int num, int pos);



/* main */
/*-------------------------------------*/

int main()
{
  char command;

  /* 初期化する */
  init_disp();
  init_buff();
  show_one_page();

  /* コマンドの入力と実行 */
  while((command=input_cmd())!=QUIT){
    exec_cmd(command);
  }

  /* 後始末する */
  clear();
  endwin();
  return 0;
}


/*-------------------------------*/
/* コマンド処理用部品 */
/*-------------------------------*/

#define CMD_MSG_LINE MSG_LINE
#define CMD_ERR_LINE MSG_LINE+1
#define CMD_LINE MSG_LINE+2

#define CMD_MSG "コマンド(A:追加 U:次貢 D:前貢 Q:終了)"
#define CMD_ERR_MSG "不適切なコマンドでした"
#define CMD_PROMPT ">>"


/* input_cmd */
/*--------------------------------------*/
/*
    書式:char input_cmd()

    機能:住所録のコマンドを入力し、妥当なコマンドであれば、入力されたコマ
           ンドを返す。もしも、不適当なコマンドであれば、再入力を要求する。

　　返値:入力されたコマンド
*/

char input_cmd()
{
  int cmd;
  int match=FALSE;
  int i;
  char buffer[BUFFER_LEN];

  /* 前処理 */
  move(CMD_MSG_LINE, 1);
  printw(CMD_MSG);

  /* コマンドの入力 */
  for(; ;){
    move(CMD_LINE, 1); /* コマンドを1文字入力する */
    clrtoeol();
    printw(CMD_PROMPT);
    getstr(buffer);
    cmd = toupper(buffer[0]);
    for(i=0; i<CMD_MAX; i++){ /* 妥当なコマンドか？ */
      if(cmd == cmd_list[i]){
        match=TRUE;
        break;
      }
    }
    if(!match){
      move(CMD_ERR_LINE, 1);
      printw(CMD_ERR_MSG);
    }else{
      break;
    }
  }

  /* 後始末 */
  move(CMD_MSG_LINE, 1);
  for(i=CMD_MSG_LINE; i<=CMD_LINE; i++){
    clrtoeol();
    addch('\n');
  }
  return(cmd);
}


/* exec_cmd */
/*----------------------------------------*/
/*
    書式: void exec_cmd(char cmd)

    機能: 変数cmdで、指定されたコマンドを実行する。

    返値: なし
*/

void exec_cmd(
              char cmd /* コマンド */
              )
{
  switch(cmd){
  case APPEND:
    data_append(); /* 追加 */
    break;
  case UP_PAGE:
    paging(UP); /* 次ページ */
    break;
  case DOWN_PAGE:
    paging(DOWN); /* 前ページ */
    break;
  default:
    break;
  }
  /* データの再表示 */
  show_one_page();
}


/*----------------------------------*/
/* 住所録管理コマンド群 */
/*----------------------------------*/

#define APPEND_LINE MSG_LINE
#define APPEND_END_LINE MSG_LINE+4

#define ERR_APPEND "データがいっぱいで、もう追加できません。"

#define WATT_MSG "確認したら、好きなキーを押してください。"


/* data_append */
/*------------------------------------------------*/
/*
     書式: void data_append(void)

     機能: 住所録データの末尾に、新しいデータを追加する。

     返値: なし
*/

void data_append(void)
{
  int i;
  char buffer[BUFFER_LEN];

  move(APPEND_LINE, 1);

  /* データがいっぱいかどうかを調べる。 */
  if(rec_qty >= RECORD_MAX){
    printw(ERR_APPEND);
    printw(WATT_MSG);
    getstr(buffer);
    return;
  }

  /* データを入力する。 */
  printw("名前 >>");
  getnstr(j_name[rec_qty], NAME_LEN);
  printw("フリガナ >>");
  getnstr(k_name[rec_qty], NAME_LEN);
  printw("住所 >>");
  getnstr(address[rec_qty], ADDR_LEN);
  printw("郵便番号 >>");
  getnstr(zip[rec_qty], ZIP_LEN);
  printw("電話番号 >>");
  getnstr(tel_no[rec_qty], TEL_LEN);

  cursor=++rec_qty;

  /* 後始末 */
  move(APPEND_LINE, 1);
  for(i=APPEND_LINE; i <= APPEND_END_LINE; i++){
    clrtoeol();
    addch('\n');
  }
}


/* paging */
/*---------------------------------*/
/*
       書式: void paging(int direction)

       機能: 注目行を前、あるいは次ページに移動する。移動できないときは、そのまま。

       返値: なし
*/

void paging(
            int direction /* ページングの方向 */
            )
{
  int next;

  if(rec_qty==0) /* データなし */
    return;

  switch(direction){
  case UP:
    next = cursor+DISP_REC_MAX;
    if(next>rec_qty)
      next=rec_qty;
    break;
  case DOWN:
    next=cursor-DISP_REC_MAX;
    if(next<=0)
      next=1;
    break;
  default:
    break;
  }
  cursor=next;
}

/* init_buff */
/*--------------------------------------*/
/*
       書式: void init_buff()

       機能: 住所録の文字列を初期化する

       返値: なし
*/

void init_buff()
{
  int i;
  // 住所録のデータの初期化
  for (i=0; i<RECORD_MAX; i++){
    k_name[i][0]='\0';
    j_name[i][0]='\0';
    zip[i][0]='\0';
    address[i][0]='\0';
    tel_no[i][0]='\0';
  }
}

/*-------------------*/
/* 画面表示用部品 */
/*-------------------*/

#define LINE "---------------------------------------------------"

#define TITLE_POS 25

#define DATA_START DATA_LINE /* データ領域の開始行 */
#define DATA_END MSG_LINE-1 /* データ領域の最終行 */
#define DATA_STEP 3 /* データの間隔 */


/* init_disp */
/*--------------------------------------------------------------*/
/*
        書式: void init_disp(void)

        機能: 画面を消去し、第1行目にタイトルを書いておく
*/

void init_disp(void)
{
  setlocale(LC_ALL,""); // for UTF-8 setting
  initscr(); // 画面初期化
  clear();
  move(TITLE_LINE, TITLE_POS);
  printw("住所管理表　第1版\n%s", LINE);
}


/* show_one_page */
/*---------------------------------------------*/
/*
       書式: void show_one_page(void)

       機能: 1ページ分の住所録データを1画面に書く。
　　　　　　 1ページには、5件分のデータを書く。

*/

void show_one_page(void)
{
  int start, end, /* 表示開始、終了レコード番号 */
    rec_number, /* 表示するレコード番号 */
    line_pos=DATA_START; /* 表示位置 */
  int i;

  /* 前処理 */
  for(i=DATA_START; i<=DATA_END; i++){
    move(i, 0);
    clrtoeol();
  }
  start=(((cursor-1)/5)*5+1);
  end=start+DISP_REC_MAX-1;
  if(end>rec_qty)
    end=rec_qty;

  /* 5件分のデータを書く。 */
  for(i=start; i<=end; i++){
    show_record(i, line_pos);
    line_pos += DATA_STEP;
  }
}


/* show_record */
/*---------------------------------------------*/

void show_record(
                 int num, /* 表示したいレコード表号 */
                 int pos /* 表示したい行位置 */
                 )
{
  int idx;

  move(pos, 1);

  /* 注目マークをつける */
  if(num==cursor)
    printw("*");
  else
    printw(" ");

  idx=num-1;
  printw("%3d %-20s TEL %-15s \n", num, j_name[idx], tel_no[idx]);
  printw("%-20s 〒%-8s %s\n", k_name[idx], zip[idx], address[idx]);
  printw("%s", LINE);
}

