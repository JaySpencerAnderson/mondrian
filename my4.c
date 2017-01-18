#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>

#define DEBUG 0
/* Maximum length of side for a Mondrian Art Puzzle */

typedef struct {
	int y, x, height, width, created, deleted, protrude;
} rectangle;

#define NOTYET -1

/* Values that affect placement of rectangles */
#define TOPEDGE 1
#define RIGHTEDGE 2
#define BOTTOMEDGE 4
#define LEFTEDGE 8
#define CENTER 16
#define nextEdge(e) (e<<=1)

#define min(x,y) (((x)<(y))?(x):(y))
#define max(x,y) (((x)>(y))?(x):(y))
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* Maximum number of factors - arbitrary limit */
#define MAXFACTORS 1000

#define EOL printf("\n")
#define isCurrent(r) (r.created != NOTYET && r.deleted == NOTYET)
#define deleteTxn(r,t) (r.deleted=t)



int area(rectangle r){
	return r.width*r.height;
}
void pop(rectangle *s){
	unsigned int k=0;
	while(s[k].width){
		k++;
	}
	s[k-1].width=s[k-1].height=0;
}
void rpush(rectangle *s, rectangle x){
	unsigned int k=0;
	while(s[k].width){
		k++;
	}
	x.deleted=NOTYET;
	x.protrude=0;
	s[k++]=x;
	s[k].width=s[k].height=0;

	return;
}
void dumprectangle(rectangle r){
	printf("%dX%d@[%d,%d] (%d,%d) %d\t",r.width, r.height, r.x, r.y, r.created, r.deleted, r.protrude);
}
void briefdumpstack(rectangle *s){
	unsigned int k=0;
	while(s[k].width){
		if(isCurrent(s[k])){
			dumprectangle(s[k]);
		}
		k++;
	}
	printf("\n");
}
void dumpstack(rectangle *s){
	unsigned int k=0;
	while(s[k].width){
		dumprectangle(s[k]);
		k++;
	}
}
rectangle initrectangle(int width, int height){
	rectangle r;
	r.x=r.y=0;
	r.width=width;
	r.height=height;
	r.created=0;
	r.deleted=NOTYET;
	return r;
}
void initstack(rectangle *s, int n){
	int i;
	for(i=0;i<n;i++){
		s[i].y=s[i].x=s[i].height=s[i].width=0;
	}
}

int bitcount(int x){
	int count=0;
	while(x){
		if(x&1){
			count++;
		}
		x>>=1;
	}
	return count;
}

int congruent(rectangle a, rectangle b){
	return min(a.height,a.width) == min(b.height,b.width) && max(a.height,a.width) == max(b.height,b.width);
}
/* Used by graph */
int horizontal(rectangle s, int y, int x){
	if(s.y == y && x >= s.x && x < s.x+s.width){
		return TRUE;
	}
	else if(s.y+s.height == y && x >= s.x && x < s.x+s.width){
		return TRUE;
	}
	return FALSE;
}
/* Used by graph */
int vertical(rectangle s, int y, int x){
	if(s.x == x && y > s.y && y <= s.y+s.height){
		return TRUE;
	}
	else if(s.x+s.width == x && y > s.y && y <= s.y+s.height){
		return TRUE;
	}
	return FALSE;
}
/* Graph the rectangle arrangement */
void graph(rectangle *s, int side){
	unsigned int row,col,i;
	unsigned int line;
	printf("{\n");
/* vertical lines take precedence since "1" cell is 1 char high and 2 char wide */
	for(row=0;row<=side;row++){
		for(col=0;col<=side;col++){
			line=0;
/* Possible values are "  " (0), "__" (1), "| " (2) or "|_" (3). */
			for(i=0;s[i].width;i++){
				if(isCurrent(s[i])){
					if(horizontal(s[i],row,col)){
						line|=1;
					}
					if(vertical(s[i],row,col)){
						line|=2;
					}
				}
			}
			
			switch(line){
			case 0: printf("  ");	break;
			case 1: printf("__");	break;
			case 2: printf("| ");	break;
			case 3: printf("|_");	break;
			default: printf("##");	break;
			}
		}
		printf("\n");
	}
	printf("}\n");
}
/* Show (including the graph) a rectangle arrangement */
void report(rectangle *s, int side){
	int i;
	unsigned int smallest,biggest,area=0;

	smallest=side*side;
	biggest=0;

	for(i=0;s[i].width;i++){
		if(isCurrent(s[i])){
			smallest=min(smallest,s[i].width*s[i].height);
			biggest=max(biggest,s[i].width*s[i].height);
		}
	}
	printf("{%d}\n",biggest-smallest);
	graph(s, side);
	printf("{\nDimensions\tLocation\n");
	for(i=0;s[i].width;i++){
		printf("%dx%d\t\t[%d,%d]\n",
			s[i].width,			s[i].height,
			s[i].x,				s[i].y);
	}
	printf("}\n");
}

unsigned int sumstack(rectangle *s){
	unsigned int sum=0;
	int i;
	for(i=0;s[i].width;i++){
		if(isCurrent(s[i])){
			sum+=s[i].width*s[i].height;
			s++;
		}
	}
	return sum;
}
unsigned int minstack(rectangle *s){
	unsigned int area=400000;
	int i;

	for(i=0;s[i].width;i++){
		if(isCurrent(s[i])){
			area=min(area,s[i].width*s[i].height);
		}
	}
	return area;
}
void rollback(rectangle *r, int txn){
	int i;

	if(txn != NOTYET){
		for(i=0;r[i].width;i++){
			if(r[i].created == txn){
				r[i].created=r[i].deleted=NOTYET;
				r[i].x=r[i].width=r[i].y=r[i].height=0;
			}
			else if(r[i].deleted == txn){
				r[i].deleted=NOTYET;
			}
		}
	}
}

int overlap(rectangle a, rectangle b){
	if((a.x < b.x+b.width && a.x+a.width > b.x) && (b.y < a.y+a.height && b.y+b.height > a.y)){
		return TRUE;
	}
	return FALSE;
}
int stackoverlap(rectangle *callstack, rectangle next){
	int i,j;
	for(i=0;callstack[i].width;i++){
		if(overlap(callstack[i], next)){
			return TRUE;
		}
	}
	return FALSE;
}
rectangle rotate(rectangle a){
	int x=a.width;
	a.width=a.height;
	a.height=x;
	return a;
}
/* Place rectangles along the outer edge of the square,
 * starting with the upper left-hand corner and proceding clockwise.
 * This way we have a checkpoint at each corner to see that we're on the right track.
 */
int buildedge(rectangle *stack, rectangle *callstack,int side, rectangle *space, int debug){
	int i,j,edge,goal,nextgoal,x,y,d,mindim,minarea,result=FALSE,spacetxn,stacktxn;


/* Find the smallest dimension - for later.
 * We don't want to leave gaps in space that are smaller than the smallest dimension. */
	mindim=side;
	minarea=side*side;
	for(i=0;stack[i].width;i++){
		mindim=min(mindim,min(stack[i].width,stack[i].height));
		minarea=min(minarea,area(stack[i]));
	}

/* Find out what has been accomplished so far in callstack since this gets called recursively */
	/* x and y are where we expect the next rectangle to appear (in clockwise fashion) */
	x=y=0;
	edge=TOPEDGE;
	i=0;
	while(edge == TOPEDGE && callstack[i].width != 0){
		if(callstack[i].x == x && callstack[i].y == y){
#if DEBUG>=2
printf("Found %dx%d rectangle at %d,%d on TOPEDGE\n",callstack[i].width,callstack[i].height,x,y);
#endif
			x+=callstack[i].width;
			if(x == side){
				nextEdge(edge);
				y=0;
#if DEBUG>=2
printf("           going to %d,%d on next edge: %d\n",x,y,edge);
#endif
			}
			i=0;
		}
		else {
			i++;
		}
	}
	while(edge == RIGHTEDGE && callstack[i].width != 0){
		if(callstack[i].x+callstack[i].width == x && callstack[i].y == y){
#if DEBUG>=2
printf("Found %dx%d rectangle at %d,%d on RIGHTEDGE\n",callstack[i].width,callstack[i].height,x,y);
#endif
			y+=callstack[i].height;
			if(y == side){
				nextEdge(edge);
				x=side;
#if DEBUG>=2
printf("           going to %d,%d on next edge: %d\n",x,y,edge);
#endif
			}
			i=0;
		}
		else {
			i++;
		}
	}
	while(edge == BOTTOMEDGE && callstack[i].width != 0){
		if(callstack[i].x+callstack[i].width == x && callstack[i].y+callstack[i].height == y){
#if DEBUG>=2
printf("Found %dx%d rectangle at %d,%d on BOTTOMEDGE\n",callstack[i].width,callstack[i].height,x,y);
#endif
			x-=callstack[i].width;
			if(x == 0){
				nextEdge(edge);
				y=side;
#if DEBUG>=2
printf("           going to %d,%d on next edge: %d\n",x,y,edge);
#endif
			}
			i=0;
		}
		else {
			i++;
		}
	}
	while(edge == LEFTEDGE && callstack[i].width != 0){
		if(callstack[i].x == x && callstack[i].y+callstack[i].height == y){
#if DEBUG>=2
printf("Found %dx%d rectangle at %d,%d on LEFTEDGE\n",callstack[i].width,callstack[i].height,x,y);
#endif
			y-=callstack[i].height;
			if(y == 0){
				nextEdge(edge);
#if DEBUG>=2
printf("           going to %d,%d on next edge: %d\n",x,y,edge);
#endif
			}
			i=0;
		}
		else {
			i++;
		}
	}
	if(edge == CENTER){
#if DEBUG>=2
printf("  Reached the center: "); dumpstack(callstack);
#endif
		if(debug){
			printf("%c[1;1H]",(char)27);
			graph(space,side);
			printf("Space: ");
			briefdumpstack(space);
			printf("%c[0J\nNarrow: %2d, Smallest: %3d%c[0J",(char)27,narrow(space),smallest(space),(char)27);
		}
		/* rectangles are placed all along the perimeter of the square.
		 * Now match will use a different strategy to match the remaining space
		 * with what remains in stack */
		if(match(stack,callstack,space,debug)){
			report(callstack,side);
			return TRUE;
		}
		return FALSE;
	}
	/* We've found out what has been placed so far.  Now we set a goal dimension
	 * for the next rectangle along the edge. */
	switch(edge){
	case TOPEDGE:
		goal=side-x;
		break;
	case RIGHTEDGE:
		goal=side-y;
		break;
	case BOTTOMEDGE:
		goal=x;
		break;
	case LEFTEDGE:
		/* Still a good assumption that callstack[0] is at 0,0 */
		goal=y-callstack[0].height;
		break;
	default:
		fprintf(stderr,"Error: buildedge has unexpected edge (b): %d\n",edge);
		exit(0);
	}
	/* If we can't find a rectangle with a dimension of goal, nextgoal is the next 
	 * largest value a dimension can have.  Why?  Because any larger than that would
	 * leave a gap smaller than mindim - which we would be unable to fill. */
	nextgoal=goal-mindim;

#if DEBUG>=2
printf("  Next open location at %d,%d\n",x,y);
#endif

	for(i=0;stack[i].width;i++){
		if(isCurrent(stack[i])){
			for(d=0;d<2;d++){	/* The d loop rotates the rectangle 90 degrees most of the time */
				switch(edge){
				case TOPEDGE:
					if(stack[i].width == goal || stack[i].width <= nextgoal){
						stack[i].x=x;
						stack[i].y=y;
						if(!stackoverlap(callstack, stack[i])){
							spacetxn=nexttransaction(space);
							stacktxn=nexttransaction(stack);
							deleteTxn(stack[i],stacktxn);
							removerectangle(space, stack[i], spacetxn);
							if(narrow(space) >= mindim && smallest(space) >= minarea){
								rpush(callstack, stack[i]);
								if(buildedge(stack, callstack, side, space, debug)){
									return TRUE;
								}
								pop(callstack);
							}
							rollback(space, spacetxn);
							rollback(stack, stacktxn);
							stack[i].x=stack[i].y=0;
						}
					}
					break;
				case RIGHTEDGE:
					if(stack[i].height == goal || stack[i].height <= nextgoal){
						stack[i].x=x-stack[i].width;
						stack[i].y=y;
						if(!stackoverlap(callstack, stack[i])){
							spacetxn=nexttransaction(space);
							stacktxn=nexttransaction(stack);
							deleteTxn(stack[i],stacktxn);
							removerectangle(space, stack[i], spacetxn);
							if(narrow(space) >= mindim && smallest(space) >= minarea){
								rpush(callstack, stack[i]);
								if(buildedge(stack, callstack, side, space, debug)){
									return TRUE;
								}
								pop(callstack);
							}
							rollback(space, spacetxn);
							rollback(stack, stacktxn);
							stack[i].x=stack[i].y=0;
						}
					}
					break;
				case BOTTOMEDGE:
					if(stack[i].width == goal || stack[i].width <= nextgoal){
						stack[i].x=x-stack[i].width;
						stack[i].y=y-stack[i].height;
						if(!stackoverlap(callstack, stack[i])){
							spacetxn=nexttransaction(space);
							stacktxn=nexttransaction(stack);
							deleteTxn(stack[i],stacktxn);
							removerectangle(space, stack[i], spacetxn);
							if(narrow(space) >= mindim && smallest(space) >= minarea){
								rpush(callstack, stack[i]);
								if(buildedge(stack, callstack, side, space, debug)){
									return TRUE;
								}
								pop(callstack);
							}
							rollback(space, spacetxn);
							rollback(stack, stacktxn);
							stack[i].x=stack[i].y=0;
						}
					}
					break;
				case LEFTEDGE:
					if(stack[i].height == goal || stack[i].height <= nextgoal){
						stack[i].x=x;
						stack[i].y=y-stack[i].height;
						if(!stackoverlap(callstack, stack[i])){
							spacetxn=nexttransaction(space);
							stacktxn=nexttransaction(stack);
							deleteTxn(stack[i],stacktxn);
							removerectangle(space, stack[i], spacetxn);
							if(narrow(space) >= mindim && smallest(space) >= minarea){
								rpush(callstack, stack[i]);
								if(buildedge(stack, callstack, side, space, debug)){
									return TRUE;
								}
								pop(callstack);
							}
							rollback(space, spacetxn);
							rollback(stack, stacktxn);
							stack[i].x=stack[i].y=0;
						}
					}
					break;
				default:
					fprintf(stderr,"Error: buildedge has unexpected edge (c): %d\n",edge);
					exit(0);
				}
/* If we rotate the first rectangle, we just have the same problem in mirror image - so don't waste the effort. */
				if(callstack[0].width != 0 && stack[i].width != stack[i].height){
					stack[i]=rotate(stack[i]);
				}
				else {
					break;
				}
			}
		}
	}
	return FALSE;
}
int populatestack(rectangle *stack, int score, int side, int rectangles){
	int offset,negative,area,mindim;
	rectangle local;

	int avg_area=(side*side)/rectangles;

	if(avg_area < 4){
		/* It's getting too small - really */
		return FALSE;
	}
	local.x=0;
	local.y=0;
	local.created=0;
	local.deleted=NOTYET;

	initstack(stack,MAXFACTORS);
	/* Generate rectangles with areas in a range of *score* around the avg area for this puzzle */
	for(offset=1;offset<=score;offset++){
		negative=offset&1;
		area=avg_area + (negative?(0-(offset>>1)):(offset>>1));
		mindim=area/side;

		if(side*(area/side) == area){
			local.width=side;
			local.height=area/side;
			rpush(stack,local);
		}

		if(area > 0){
			for(local.width=side-mindim;local.width>=area/local.width;local.width--){
				if(local.width*(area/local.width) == area){
					local.height=area/local.width;
					rpush(stack,local);
				}
			}
		}
	}
	return TRUE;
}

int solve(int side,int rectangles,int score, int debug){
	rectangle stack[MAXFACTORS],callstack[MAXFACTORS];
	rectangle space[MAXFACTORS];
	rectangle universe;

	/* Populate the stack array with all rectangles within the range of score about the average size */
	if(!populatestack(stack, score, side, rectangles)){
		return FALSE;
	}
	/* At this point, we have a stack of rectangles within a range of offset, with at least the required area */
	if(sumstack(stack) >= side*side){
		initstack(callstack,MAXFACTORS);
		initstack(space,MAXFACTORS);
		if(debug){
			printf("%c[2J",(char)27);

			printf("Potential rectangles: \n");
			dumpstack(stack);

		}

		/* Initialize space (not occupied by a rectangle) to be side by side
		 * where side is the height/width of the square into which the rectangles fit. */
		universe.width=universe.height=side;
		universe.x=universe.y=0;
		universe.created=0;
		universe.deleted=NOTYET;
		rpush(space, universe);

		if(buildedge(stack,callstack,side,space,debug)){
			return TRUE;
		}
	}

	return FALSE;
}



int containsPoint(rectangle a, int x, int y){
	return a.x <= x && a.y <= y && a.x+a.width > x && a.y+a.height > y;
}
int containsRectangle(rectangle a, rectangle b){
	return containsPoint(a, b.x, b.y) && containsPoint(a, b.x+b.width-1, b.y) && containsPoint(a, b.x, b.y+b.height-1) && containsPoint(a, b.x+b.width-1, b.y+b.height-1);
}
int areEqual(rectangle a, rectangle b){
	return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
}
int nexttransaction(rectangle *r){
	int i,n=NOTYET;

	for(i=0;r[i].width;i++){
		n=max(n,max(r[i].created,r[i].deleted));
	}
	return n+1;
}
void splitrectanglevertically(rectangle *space, int i, int x, int txn){
	rectangle left, right;
	left=right=space[i];
	right.x=x;
	left.width=right.x-left.x;
	right.width-=left.width;
	left.created=right.created=space[i].deleted=txn;

	rpush(space,left);
	rpush(space,right);
}
void splitrectanglehorizontally(rectangle *space, int i, int y, int txn){
	rectangle top, bottom;
	top=bottom=space[i];
	bottom.y=y;
	top.height=bottom.y-top.y;
	bottom.height-=top.height;
	top.created=bottom.created=space[i].deleted=txn;

	rpush(space,top);
	rpush(space,bottom);
}
int smallest(rectangle *space){
	int i,j,smallest;
	rectangle current;

	smallest=0;
	for(i=0;space[i].width;i++){
		if(isCurrent(space[i])){
			current=space[i];
			for(j=0;space[j].width;j++){
				if(isCurrent(space[j]) && i != j){
					if(current.x+current.width == space[j].x
					&& space[j].y <= current.y && space[j].y+space[j].height >= current.y+current.height){
						current.width+=space[j].width;
					}
					else if(space[j].x+space[j].width == current.x
					&& space[j].y <= current.y && space[j].y+space[j].height >= current.y+current.height){
						current.x=space[j].x;
						current.width+=space[j].width;
					}
					else if(current.y+current.height == space[j].y
					&& space[j].x <= current.x && space[j].x+space[j].width >= current.x+current.width){
						current.height+=space[j].height;
					}
					else if(space[j].y+space[j].height == current.y
					&& space[j].x <= current.x && space[j].x+space[j].width >= current.x+current.width){
						current.y=space[j].y;
						current.height+=space[j].height;
					}
				}
			}
			if(smallest == 0){
				smallest=current.width * current.height;
			}
			else if(smallest > current.width * current.height){
				smallest=current.width * current.height;
			}
		}
	}
	return smallest;
}
int narrow(rectangle *space){
	int i,j;
	rectangle smallest,current;

	smallest.width=0;
	for(i=0;space[i].width;i++){
		current=space[i];
		if(isCurrent(current)){
			for(j=0;space[j].width;j++){
				if(isCurrent(space[j]) && i != j){
					if(current.width <= current.height
					&& current.x+current.width == space[j].x
					&& space[j].y <= current.y && space[j].y+space[j].height >= current.y+current.height){
						current.width+=space[j].width;
					}
					else if(current.width <= current.height
					&& space[j].x+space[j].width == current.x
					&& space[j].y <= current.y && space[j].y+space[j].height >= current.y+current.height){
						current.x=space[j].x;
						current.width+=space[j].width;
					}

					if(current.width >= current.height
					&& current.y+current.height == space[j].y
					&& space[j].x <= current.x && space[j].x+space[j].width >= current.x+current.width){
						current.height+=space[j].height;
					}
					else if(current.width >= current.height
					&& space[j].y+space[j].height == current.y
					&& space[j].x <= current.x && space[j].x+space[j].width >= current.x+current.width){
						current.y=space[j].y;
						current.height+=space[j].height;
					}
				}
			}
			if(smallest.width == 0){
				smallest=current;
			}
			else if(min(smallest.width,smallest.height) > min(current.width,current.height)){
				smallest=current;
			}
		}
	}
	return min(smallest.width,smallest.height);
}
int notEmpty(rectangle *space, int debug){
	int i,count;

	for(i=0,count=0;space[i].width;i++){
		if(isCurrent(space[i])){
			count++;
#if DEBUG>=2
			printf("Is current: ");dumprectangle(space[i]);printf("\n");
#endif
		}
	}
	return count;
}
/* Also see above in *narrow* */
int isAdjacent(rectangle r, rectangle s){
	/* is s adjacent to r?  If so, return on which edge (of r). Otherwise, return NOTYET. */
/* touching at the corners doesn't count */
	if(r.y == s.y+s.height && r.x < s.x+s.width && s.x < r.x+r.width){
		return TOPEDGE;
	}
	if(s.x == r.x+r.width && r.y < s.y+s.height && s.y < r.y+r.height){
		return RIGHTEDGE;
	}
	if(s.y == r.y+r.height && r.x < s.x+s.width && s.x < r.x+r.width){
		return BOTTOMEDGE;
	}
	if(r.x == s.x+s.width && r.y < s.y+s.height && s.y < r.y+r.height){
		return LEFTEDGE;
	}
	
	return NOTYET;
}

int adjacentrectangle(rectangle *space, int k, int k0, int debug){
	int i,edge;
	for(i=k0+1;space[i].width;i++){
		if(i != k && isCurrent(space[i])){
			if(isAdjacent(space[k],space[i]) != NOTYET){
				return i;
			}
		}
	}
	return NOTYET;
}
int expanse(rectangle *space, int j, int d){ /* Returns how far space[j] can expand in the d direction */
	int extent,k,giveUp,distance;
	rectangle result=space[j];

	extent=0;
	giveUp=FALSE;
	distance=0;
#if DEBUG>=2
printf("expanse - looking in direction: %d from ",d);dumprectangle(space[j]);EOL;
#endif
	if(d == TOPEDGE || d == BOTTOMEDGE){
		while(extent < space[j].width && !giveUp){
			giveUp=TRUE;
			for(k=0;space[k].width;k++){
				if(k != j && isCurrent(space[k]) && isAdjacent(space[j],space[k]) == d){
					if(space[j].x+extent == space[k].x){
#if DEBUG>=2
printf("in expanse - next adjacent (exact) is ");dumprectangle(space[k]);EOL;
#endif
						extent+=space[k].width;
						if(distance == 0){
							distance=expanse(space,k,d);
						}
						else {
							distance=min(distance,expanse(space,k,d));
						}
#if DEBUG>=2
printf("in expanse - distance is %d\n",distance);
#endif
						giveUp=FALSE;
					}
					else if(space[j].x+extent > space[k].x && space[j].x+extent < space[k].x+space[k].width){
#if DEBUG>=2
printf("in expanse - next adjacent (not exact) is ");dumprectangle(space[k]);EOL;
#endif
						extent=space[k].x+space[k].width-space[j].x;
						if(distance == 0){
							distance=expanse(space,k,d);
						}
						else {
							distance=min(distance,expanse(space,k,d));
						}
#if DEBUG>=2
printf("in expanse - distance is %d\n",distance);
#endif
						giveUp=FALSE;
					}
				}
			}
		}
		if(extent < space[j].width){
			return 0;
		}
		return space[j].height+distance;
	}
	else if(d == LEFTEDGE || d == RIGHTEDGE){
		while(extent < space[j].height && !giveUp){
			giveUp=TRUE;
			for(k=0;space[k].width;k++){
				if(k != j && isCurrent(space[k]) && isAdjacent(space[j],space[k]) == d){
					if(space[j].y+extent == space[k].y){
#if DEBUG>=2
printf("in expanse - next adjacent (exact) is ");dumprectangle(space[k]);EOL;
#endif
						extent+=space[k].height;
						if(distance == 0){
							distance=expanse(space,k,d);
						}
						else {
							distance=min(distance,expanse(space,k,d));
						}
#if DEBUG>=2
printf("in expanse - distance is %d\n",distance);
#endif
						giveUp=FALSE;
					}
					else if(space[j].y+extent > space[k].y && space[j].y+extent < space[k].y+space[k].height){
#if DEBUG>=2
printf("in expanse - next adjacent (not exact) is ");dumprectangle(space[k]);EOL;
#endif
						extent=space[k].y+space[k].height-space[j].y;
						if(distance == 0){
							distance=expanse(space,k,d);
						}
						else {
							distance=min(distance,expanse(space,k,d));
						}
#if DEBUG>=2
printf("in expanse - distance is %d\n",distance);
#endif
						giveUp=FALSE;
					}
				}
			}
		}
		if(extent < space[j].height){
			return 0;
		}
		return space[j].width+distance;
	}
	return 0;
}
int match(rectangle *stack, rectangle *callstack, rectangle *space, int debug){
	int i,j,k,d,goal,mn;
	int height;
	int spacetxn, stacktxn, calltxn;
	int map;
	rectangle r;

	for(i=0,goal=0;space[i].width;i++){
		if(isCurrent(space[i])){
			goal+=space[i].width*space[i].height;
		}
	}
/* If no space left */
	if(goal == 0){
#if DEBUG>=2
printf("Looks like no space is left - in match\n");
#endif
		return assert(notEmpty(space,debug),FALSE);
	}
	mn=minstack(stack);
/* If a tiny bit of space left */
	if(goal < mn){
#if DEBUG>=2
printf("Looks like not enough space is left - in match\n");
#endif
		/* The goal (space available) is smaller than any rectangle left in the stack */
		return assert(notEmpty(space,debug),FALSE);
	}

	spacetxn=nexttransaction(space);
	stacktxn=nexttransaction(stack);
	calltxn=nexttransaction(callstack);
	for(j=0;space[j].width;j++){
		for(i=0;stack[i].width;i++){
			if(isCurrent(stack[i]) && isCurrent(space[j])){
/* Just try to do a one to one match between space rectangles with no adjacent rectangles and stack rectangles. */
				if(congruent(space[j], stack[i]) && adjacentrectangle(space,j,NOTYET,debug) == NOTYET){
#if DEBUG>=2
printf("in match - deleting ");dumprectangle(space[j]);printf(" since no adjacent space and exact match\n");
#endif
					r=space[j];
					r.created=calltxn;
					rpush(callstack, r);
					deleteTxn(stack[i],stacktxn);
					deleteTxn(space[j],spacetxn);
				}
			}
		}
	}
	if(!notEmpty(space,debug)){
		return TRUE;
	}
#if DEBUG>=2
printf("in match - continuing since there is still some space left\n");
#endif

/* Separate loop to enforce priorities */
	rectangle e;
	for(j=0;space[j].width;j++){
		if(isCurrent(space[j])){
			e=space[j];
			for(k=0,map=0;space[k].width;k++){
				if(k != j && isCurrent(space[k])){
					d=isAdjacent(space[j], space[k]);
					if(d != NOTYET){
						map|=d;
					}
				}
			}
			if(bitcount(map) == 1){	/* space[j] has adjacent space on only one side */
				if(map == TOPEDGE || map == BOTTOMEDGE){
					e.height=expanse(space,j,map);
				}
				else if(map == LEFTEDGE || map == RIGHTEDGE){
					e.width=expanse(space,j,map);
				}
				for(i=0;stack[i].width;i++){
					if(isCurrent(stack[i])){
						if(congruent(e, stack[i])){
							e.created=calltxn;
							rpush(callstack, e);
							deleteTxn(stack[i],stacktxn);
							if(!removerectangle(space, e, spacetxn)){
								printf("Logic error in match/expanse.  Terminating\n");
								exit(0);
							}
							if(match(stack,callstack,space,debug)){
								return TRUE;
							}
							else {
								rollback(stack,stacktxn);
								rollback(callstack,calltxn);
								rollback(space,spacetxn);
								return FALSE;
							}
						}
						else if(congruent(space[j], stack[i])){
							r=space[j];
							r.created=calltxn;
							rpush(callstack, r);
							deleteTxn(stack[i],stacktxn);
							if(!removerectangle(space, r, spacetxn)){
								printf("Logic error in match/expanse.  Terminating\n");
								exit(0);
							}
							if(match(stack,callstack,space,debug)){
								return TRUE;
							}
							else {
								rollback(stack,stacktxn);
								rollback(callstack,calltxn);
								rollback(space,spacetxn);
								return FALSE;
							}
						}
					}
				}
			}
		}
	}

	if(notEmpty(space,debug)){
		rollback(stack,stacktxn);
		rollback(callstack,calltxn);
		rollback(space,spacetxn);
		return FALSE;
	}

	return TRUE;
}
int removerectangle(rectangle *space, rectangle r, int ntxn){
	int i,status=TRUE;

	for(i=0;space[i].width;i++){
		if(space[i].deleted == NOTYET){
			if(areEqual(space[i], r)){
				/* They are the same. */
				space[i].deleted=ntxn;
				return TRUE;
			}
			else if(containsRectangle(space[i], r)){
				if(r.x > space[i].x){
					/* split space along left side of r */
					splitrectanglevertically(space, i, r.x, ntxn);
					/* Since the new rectangles are at the end of space, we can just continue with the loop */
				}
				else if(r.y > space[i].y){
					/* split space along top of r */
					splitrectanglehorizontally(space, i, r.y, ntxn);
				}
				else if(r.x+r.width < space[i].x+space[i].width){
					/* Split space just past right side of r */ 
					splitrectanglevertically(space, i, r.x+r.width, ntxn);
				}
				else if(r.y+r.height < space[i].y+space[i].height){
					/* split space along top of r */
					splitrectanglehorizontally(space, i, r.y+r.height, ntxn);
				}
			}
			else if(overlap(space[i], r)){	/* we have to split both */
				/* containsRectangle(r, space[i]) is a special case of overlap.  I'll merge the two. */
				rectangle aux;
				/* aux is going to be a subset of space[i].  We may have to create as many as 4 extra rectangles from r */

				if(r.x < space[i].x){
					/* Break off rectangle to the left */
					aux=r;
					aux.width=space[i].x-r.x;
					/* Remove aux from r */
					r.x+=aux.width;
					r.width-=aux.width;
					/* Remove aux from available space */
					if(!removerectangle(space,aux,ntxn)){
						return FALSE;
					}
				}
				if(r.x+r.width > space[i].x+space[i].width){
					/* Break off rectangle to the right */
					aux=r;
					aux.x=space[i].x+space[i].width;
					aux.width=r.x+r.width-aux.x;
					/* Remove aux from r */
					r.width-=aux.width;
					/* Remove aux from available space */
					if(!removerectangle(space,aux,ntxn)){
						return FALSE;
					}
				}
				if(r.y < space[i].y){
					/* Break off rectangle above */
					aux=r;
					aux.height=space[i].y-aux.y;
					/* Remove aux from r */
					r.y+=aux.height;
					r.height-=aux.height;
					/* Remove aux from available space */
					if(!removerectangle(space,aux,ntxn)){
						return FALSE;
					}
				}
				if(r.y+r.height > space[i].y+space[i].height){
					/* Break off rectangle below */
					aux=r;
					aux.y=space[i].y+space[i].height;
					aux.height=r.y+r.height-aux.y;
					/* Remove aux from r */
					r.height-=aux.height;
					/* Remove aux from available space */
					if(!removerectangle(space,aux,ntxn)){
						return FALSE;
					}
				}
				if(areEqual(space[i], r)){
					/* They are the same - now, after r has been trimmed. */
					space[i].deleted=ntxn;
					return TRUE;
				}
				else {
					/* This will happen if r partially overlapped space[i].
					 * Basically restart and space[i] will be split (by earlier logic)
					 * to fit r. */
					if(!removerectangle(space,r,ntxn)){
						return FALSE;
					}
					return TRUE;
				}
			}
		}
	}
/* I think there is still a case if it falls under overlap or r contains space
* that some parts of r could be unaccounted for by space */
/* Could be solved with space and anti space, aka the rectangle stack */
	return TRUE;
}
int placeonAalignB(rectangle *space, rectangle r, rectangle s, int on, int align){
	int i;

/* Not planning to validate that placement doesn't result in overlap */

	/* Validate on and align - I could screw up.  Align CENTER works either way. */
	if((on == TOPEDGE || on == BOTTOMEDGE) && (align == TOPEDGE || align == BOTTOMEDGE)){
		printf("on and align have to be at right angles\n");
		return FALSE;
	}
	if((on == RIGHTEDGE || on == LEFTEDGE) && (align == RIGHTEDGE || align == LEFTEDGE)){
		printf("on and align have to be at right angles\n");
		return FALSE;
	}

	/* Find r in space */
	for(i=0;space[i].width;i++){
		if(isCurrent(space[i])){
			if(min(space[i].width,space[i].height) == min(r.width,r.height)
			&& max(space[i].width,space[i].height) == max(r.width,r.height)){
				/* Found it */
				if(on == TOPEDGE){
					s.y=space[i].y-s.height;
				}
				else if(on == LEFTEDGE){
					s.x=space[i].x-s.width;
				}
				else if(on == BOTTOMEDGE){
					s.y=space[i].y+space[i].height;
				}
				else if(on == RIGHTEDGE){
					s.x=space[i].x+space[i].width;
				}
				else {
					printf("Must be on top, bottom, right or left, not %d\n",on);
					return FALSE;
				}
				/* Align */
				if(align == TOPEDGE){
					s.y=space[i].y;
				}
				else if(align == LEFTEDGE){
					s.x=space[i].x;
				}
				else if(align == BOTTOMEDGE){
					s.y=space[i].y+space[i].height-s.height;
				}
				else if(align == RIGHTEDGE){
					s.x=space[i].x+space[i].width-s.width;
				}
				else if(align == CENTER){
					if(on == TOPEDGE || on == BOTTOMEDGE){
						s.x=space[i].x+(space[i].width/2)-(s.width/2);
					}
					else if(on == LEFTEDGE || on == RIGHTEDGE){
						s.y=space[i].y+(space[i].height/2)-(s.height/2);
					}
				}
				else {
					printf("Must align center, top, bottom, right or left, not %d\n",align);
					return FALSE;
				}
				rpush(space, s);
				return TRUE;
			}
		}
	}
}
void placeat(rectangle *space, rectangle r, int x, int y){
	r.x=x;
	r.y=y;

	rpush(space, r);
	return;
}
int testsamerectangleset(rectangle *a, rectangle *b){
	int i,j,found;

	/* Is everything in a found in b? */
printf("Comparing a\n");
	for(i=0,found=FALSE;a[i].width;i++){
		if(isCurrent(a[i])){
dumprectangle(a[i]);EOL;
			for(j=0,found=FALSE;b[j].width;j++){
				if(isCurrent(b[j])){
					if(congruent(a[i], b[j])){
						found=TRUE;
						break;
					}
				}
			}
		}
		if(!found){
			return FALSE;
		}
	}
	/* Is everything in b found in a? */
printf("Comparing b\n");
	for(i=0,found=FALSE;b[i].width;i++){
		if(isCurrent(b[i])){
dumprectangle(b[i]);EOL;
			for(j=0,found=FALSE;a[j].width;j++){
				if(isCurrent(a[j])){
					if(congruent(b[i], a[j])){
						found=TRUE;
						break;
					}
				}
			}
		}
		if(!found){
			return FALSE;
		}
	}
	return TRUE;
}
void testdata(int debug){
	rectangle stack[MAXFACTORS];
	int side,score,rectangles;
	int i,j,k,l,height,found;

printf("Testing generation of data \n");
	rectangles=12;

	side=20;
	{
		score=13;
		{
			populatestack(stack, score, side, rectangles);
			for(i=0;stack[i].width;i++){
				for(j=i+1;stack[j].width;j++){
					if(stack[i].width == stack[j].width){
						height=stack[i].height+stack[j].height;
						for(k=0;stack[k].width;k++){
							for(l=k+1;stack[l].width;l++){
								if(k != i && k != j && l != i && l != j
								&& (height == stack[k].height || height == stack[k].width)
								&& (height == stack[l].height || height == stack[l].width)){
									found=1;
									printf("Side: %d, Score: %d: ",side,score);
									dumprectangle(stack[i]);
									dumprectangle(stack[j]);
									dumprectangle(stack[k]);
									dumprectangle(stack[l]);printf("\n");
								}
							}
						}
					}
				}
			}
			dumpstack(stack);
		}
	}
}
void testmatch(int debug){
	int i,testcase;
	rectangle stack[MAXFACTORS], callstack[MAXFACTORS], space[MAXFACTORS], bogus, r, s;
/* Initializations */
	int score, side, rectangles;
	char *description;
	score=13, side=20, rectangles=12;

	printf("Testing match\n");
	bogus.width=bogus.height=1;
	bogus.x=bogus.y=0;
	bogus.created=0; bogus.deleted=NOTYET;

	testcase=0;

	if(!populatestack(stack, score, side, rectangles)){
		printf("testmatch failed\n");
		return;
	}
/*
 * For test case where 2, 3 and 4 rectangles are arrayed in a rectangle.
 * Side: 20, Score: 13: 7X5@[0,0] (0,-1)	7X4@[0,0] (0,-1)	9X4@[0,0] (0,-1)	9X3@[0,0] (0,-1)	
 * Side: 20, Score: 13: 9X4@[0,0] (0,-1)	9X3@[0,0] (0,-1)	7X5@[0,0] (0,-1)	7X4@[0,0] (0,-1)	
 * 11X3	17X2	16X2	8X4	7X5	18X2	12X3	9X4	6X6	15X2	10X3	6X5	19X2	
 * 14X2	7X4	13X3	9X3	
 */
	printf("Potential rectangles\n");
	briefdumpstack(stack);
/* End of initializations */

/* Test case 0: no rectangles */
	initstack(callstack,MAXFACTORS);
	initstack(space,MAXFACTORS);
	description="no rectangles";
	populatestack(stack, score, side, rectangles);

	if(assert(match(stack, callstack, space, debug), TRUE)){
		printf("Test case %d as expected: %s\n", testcase, description);
	}
	else {
		printf("Test case %d NOT as expected: %s\n", testcase, description);
	}

/* Test case 1: 1 rectangle, no match */
	testcase++;
	initstack(callstack,MAXFACTORS);
	initstack(space,MAXFACTORS);
	description="1 rectangle, no match";
	populatestack(stack, score, side, rectangles);

	placeat(space, bogus, 10, 10);

	if(assert(match(stack, callstack, space, debug), FALSE)){
		printf("Test case %d as expected: %s\n", testcase, description);
	}
	else {
		printf("Test case %d NOT as expected: %s\n", testcase, description);
	}

/* Test case 2: 1 rectangle, match */
	testcase++;
	initstack(callstack,MAXFACTORS);
	initstack(space,MAXFACTORS);
	description="1 rectangle, match";
	populatestack(stack, score, side, rectangles);

	placeat(space, stack[3], 10, 10);

	if(debug)graph(space, side);
	if(assert(match(stack, callstack, space, debug), TRUE)){
		printf("Test case %d as expected: %s\n", testcase, description);
	}
	else {
		printf("Test case %d NOT as expected: %s\n", testcase, description);
	}
	if(debug)graph(space, side);

/* Test case 3: 2 non-adjacent rectangles, match one */
	testcase++;
	initstack(callstack,MAXFACTORS);
	initstack(space,MAXFACTORS);
	description="2 non-adjacent rectangles, match one";
	populatestack(stack, score, side, rectangles);

	placeat(space, bogus, 1, 1);
	placeat(space, stack[3], 10, 10);
	if(debug)graph(space, side);

	/* FALSE because bogus should remain unmatched */
	if(assert(match(stack, callstack, space, debug), FALSE)){
		printf("Test case %d as expected: %s\n", testcase, description);
	}
	else {
		printf("Test case %d NOT as expected: %s\n", testcase, description);
	}
	if(debug)graph(space, side);

/* Test case 4: 2 non-adjacent rectangles, match both */
	testcase++;
	initstack(callstack,MAXFACTORS);
	initstack(space,MAXFACTORS);
	description="2 non-adjacent rectangles, match both";
	populatestack(stack, score, side, rectangles);

	placeat(space, stack[2], 1, 1);
	placeat(space, stack[3], 1, 11);
	if(debug)graph(space, side);

	if(assert(match(stack, callstack, space, debug), TRUE)){
		printf("Test case %d as expected: %s\n", testcase, description);
	}
	else {
		printf("Test case %d NOT as expected: %s\n", testcase, description);
	}
	if(debug)graph(space, side);

/*
 * For test cases where 2, 3 and 4 rectangles are arrayed in a rectangle.
 * Side: 20, Score: 13: 7X5@[0,0] (0,-1)	7X4@[0,0] (0,-1)	9X4@[0,0] (0,-1)	9X3@[0,0] (0,-1)	
 * Side: 20, Score: 13: 9X4@[0,0] (0,-1)	9X3@[0,0] (0,-1)	7X5@[0,0] (0,-1)	7X4@[0,0] (0,-1)	
 * 11X3	17X2	16X2	8X4	7X5	18X2	12X3	9X4	6X6	15X2	10X3	6X5	19X2	
 * 14X2	7X4	13X3	9X3	
 */
/* Test case 5: 2 adjacent rectangles of space forming a rectangle.  One rectangle from stack matches both cumulatively. */
	testcase++;
	initstack(callstack,MAXFACTORS);
	initstack(space,MAXFACTORS);
	description="2 adjacent rectangles of space forming a rectangle.  One rectangle from stack matches both cumulatively.";
	populatestack(stack, score, side, rectangles);

	/* These should match the 12x3 rectangle */
	r=initrectangle(5,3);
	s=initrectangle(7,3);

	placeat(space, r, 1, 1);
	placeonAalignB(space, r, s, RIGHTEDGE, TOPEDGE);
	if(debug)graph(space, side);

	if(assert(match(stack, callstack, space, debug), TRUE)){
		printf("Test case %d as expected: %s\n", testcase, description);
	}
	else {
		printf("Test case %d NOT as expected: %s\n", testcase, description);
	}
	if(debug)graph(space, side);


/* Test case 6: 2 adjacent rectangles in L, match both */
	testcase++;
	initstack(callstack,MAXFACTORS);
	initstack(space,MAXFACTORS);
	description="2 adjacent rectangles in L, match both";
	populatestack(stack, score, side, rectangles);

	placeat(space, stack[2], 1, 1);
	placeonAalignB(space, stack[2], stack[1], BOTTOMEDGE, LEFTEDGE);
	if(debug)graph(space, side);

	if(assert(match(stack, callstack, space, debug), TRUE)){
		printf("Test case %d as expected: %s\n", testcase, description);
	}
	else {
		printf("Test case %d NOT as expected: %s\n", testcase, description);
	}
	if(debug)graph(space, side);

/* Test case 7: 2 adjacent rectangles in T, match both */
	testcase++;
	initstack(callstack,MAXFACTORS);
	initstack(space,MAXFACTORS);
	description="2 adjacent rectangles in T, match both";
	populatestack(stack, score, side, rectangles);

	placeat(space, stack[10], 1, 1);
	placeonAalignB(space, stack[10], stack[8], BOTTOMEDGE, CENTER);
	if(debug)graph(space, side);

	if(assert(match(stack, callstack, space, debug), TRUE)){
		printf("Test case %d as expected: %s\n", testcase, description);
	}
	else {
		printf("Test case %d NOT as expected: %s\n", testcase, description);
	}
	if(debug)graph(space, side);


/*
 * For test cases where 2, 3 and 4 rectangles are arrayed in a rectangle.
 * Side: 20, Score: 13: 7X5@[0,0] (0,-1)	7X4@[0,0] (0,-1)	9X4@[0,0] (0,-1)	9X3@[0,0] (0,-1)	
 * Side: 20, Score: 13: 9X4@[0,0] (0,-1)	9X3@[0,0] (0,-1)	7X5@[0,0] (0,-1)	7X4@[0,0] (0,-1)	
 * 11X3	17X2	16X2	8X4	7X5	18X2	12X3	9X4	6X6	15X2	10X3	6X5	19X2	
 * 14X2	7X4	13X3	9X3	
 */
/* Test case 8: 2 adjacent rectangles forming a rectangle */
	testcase++;
	initstack(callstack,MAXFACTORS);
	initstack(space,MAXFACTORS);
	description="2 adjacent rectangles forming a rectangle";
	populatestack(stack, score, side, rectangles);

	placeat(space, stack[4], 1, 1);
	placeonAalignB(space, stack[4], stack[14], BOTTOMEDGE, LEFTEDGE);
	if(debug)graph(space, side);

	if(assert(match(stack, callstack, space, debug), TRUE)){
		printf("Test case %d as expected: %s\n", testcase, description);
	}
	else {
		printf("Test case %d NOT as expected: %s\n", testcase, description);
	}
	if(debug)graph(space, side);

/* Test case 9: 3 adjacent rectangles forming a rectangle */
	testcase++;
	initstack(callstack,MAXFACTORS);
	initstack(space,MAXFACTORS);
	description="3 adjacent rectangles forming a rectangle";
	populatestack(stack, score, side, rectangles);

	placeat(space, stack[4], 6, 1);
	placeonAalignB(space, stack[4], stack[14], BOTTOMEDGE, LEFTEDGE);
	placeonAalignB(space, stack[4], rotate(stack[7]), LEFTEDGE, TOPEDGE);
	if(debug)graph(space, side);

	if(assert(match(stack, callstack, space, debug), TRUE)){
		printf("Test case %d as expected: %s\n", testcase, description);
	}
	else {
		printf("Test case %d NOT as expected: %s\n", testcase, description);
	}
	if(debug)graph(space, side);

/* Test case 10: 3 adjacent rectangles forming a rectangle */
	testcase++;
	initstack(callstack,MAXFACTORS);
	initstack(space,MAXFACTORS);
	description="3 adjacent rectangles forming a rectangle (B)";
	populatestack(stack, score, side, rectangles);

	placeat(space, stack[4], 6, 1);
	placeonAalignB(space, stack[4], stack[14], BOTTOMEDGE, LEFTEDGE);
	placeonAalignB(space, stack[4], rotate(stack[7]), RIGHTEDGE, TOPEDGE);
	if(debug)graph(space, side);

	if(assert(match(stack, callstack, space, debug), TRUE)){
		printf("Test case %d as expected: %s\n", testcase, description);
	}
	else {
		printf("Test case %d NOT as expected: %s\n", testcase, description);
	}
	if(debug)graph(space, side);

/* Test case 11: 4 adjacent rectangles forming a rectangle */
	testcase++;
	initstack(callstack,MAXFACTORS);
	initstack(space,MAXFACTORS);
	description="4 adjacent rectangles forming a rectangle";
	populatestack(stack, score, side, rectangles);

	placeat(space, stack[4], 6, 1);
	placeonAalignB(space, stack[4], stack[14], BOTTOMEDGE, LEFTEDGE);
	placeonAalignB(space, stack[4], rotate(stack[7]), RIGHTEDGE, TOPEDGE);
	placeonAalignB(space, stack[4], rotate(stack[16]), LEFTEDGE, TOPEDGE);
	if(debug)graph(space, side);

	if(assert(match(stack, callstack, space, debug), TRUE)){
		printf("Test case %d as expected: %s\n", testcase, description);
	}
	else {
		printf("Test case %d NOT as expected: %s\n", testcase, description);
	}
	if(debug)graph(space, side);

/* Test case 12: 4 adjacent rectangles forming a rectangle */
	testcase++;
	initstack(callstack,MAXFACTORS);
	initstack(space,MAXFACTORS);
	description="4 adjacent rectangles forming a rectangle (B)";
	populatestack(stack, score, side, rectangles);

	placeat(space, rotate(stack[7]), 1, 6);						/* 4x9 */
	placeonAalignB(space, stack[7], rotate(stack[16]), RIGHTEDGE, TOPEDGE);	/* 3x9 */
	placeonAalignB(space, stack[7], stack[4], TOPEDGE, LEFTEDGE);	/* 7x5 */
	placeonAalignB(space, stack[7], stack[14], BOTTOMEDGE, LEFTEDGE);	/* 7x4 */
	if(debug)graph(space, side);

	if(assert(match(stack, callstack, space, debug), TRUE)){
		printf("Test case %d as expected: %s\n", testcase, description);
	}
	else {
		printf("Test case %d NOT as expected: %s\n", testcase, description);
	}
	if(debug)graph(space, side);
}
int assert(int a, int b){
	return a == b;
}
void testoverlap(int debug){
	rectangle s,r,space[MAXFACTORS];

	printf("Test TRUE overlaps\n");

	r.x=0;	r.y=0; r.width=6; r.height=6;	r.created=0;
	s.x=3;	s.y=3; s.width=6; s.height=6;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	if(debug || (!assert(overlap(r, s), TRUE) || !assert(overlap(s, r), TRUE))){
		graph(space, 9);
		printf("Overlap %s\n",overlap(r, s)?"TRUE":"FALSE");
		printf("Overlap %s\n",overlap(s, r)?"TRUE":"FALSE");
	}

	r.x=0;	r.y=3; r.width=6; r.height=6;	r.created=0;
	s.x=3;	s.y=0; s.width=6; s.height=6;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	if(debug || (!assert(overlap(r, s), TRUE) || !assert(overlap(s, r), TRUE))){
		graph(space, 9);
		printf("Overlap %s\n",overlap(r, s)?"TRUE":"FALSE");
		printf("Overlap %s\n",overlap(s, r)?"TRUE":"FALSE");
	}

	r.x=0;	r.y=0; r.width=6; r.height=9;	r.created=0;
	s.x=3;	s.y=3; s.width=6; s.height=3;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	if(debug || (!assert(overlap(r, s), TRUE) || !assert(overlap(s, r), TRUE))){
		graph(space, 9);
		printf("Overlap %s\n",overlap(r, s)?"TRUE":"FALSE");
		printf("Overlap %s\n",overlap(s, r)?"TRUE":"FALSE");
	}

	r.x=3;	r.y=0; r.width=6; r.height=9;	r.created=0;
	s.x=0;	s.y=3; s.width=6; s.height=3;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	if(debug || (!assert(overlap(r, s), TRUE) || !assert(overlap(s, r), TRUE))){
		graph(space, 9);
		printf("Overlap %s\n",overlap(r, s)?"TRUE":"FALSE");
		printf("Overlap %s\n",overlap(s, r)?"TRUE":"FALSE");
	}

	r.x=0;	r.y=0; r.width=9; r.height=6;	r.created=0;
	s.x=3;	s.y=3; s.width=3; s.height=6;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	if(debug || (!assert(overlap(r, s), TRUE) || !assert(overlap(s, r), TRUE))){
		graph(space, 9);
		printf("Overlap %s\n",overlap(r, s)?"TRUE":"FALSE");
		printf("Overlap %s\n",overlap(s, r)?"TRUE":"FALSE");
	}

	r.x=0;	r.y=3; r.width=9; r.height=6;	r.created=0;
	s.x=3;	s.y=0; s.width=3; s.height=6;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	if(debug || (!assert(overlap(r, s), TRUE) || !assert(overlap(s, r), TRUE))){
		graph(space, 9);
		printf("Overlap %s\n",overlap(r, s)?"TRUE":"FALSE");
		printf("Overlap %s\n",overlap(s, r)?"TRUE":"FALSE");
	}

	r.x=0;	r.y=0; r.width=3; r.height=9;	r.created=0;
	s.x=0;	s.y=3; s.width=6; s.height=3;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	if(debug || (!assert(overlap(r, s), TRUE) || !assert(overlap(s, r), TRUE))){
		graph(space, 9);
		printf("Overlap %s\n",overlap(r, s)?"TRUE":"FALSE");
		printf("Overlap %s\n",overlap(s, r)?"TRUE":"FALSE");
	}

	r.x=0;	r.y=0; r.width=9; r.height=3;	r.created=0;
	s.x=3;	s.y=0; s.width=3; s.height=6;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	if(debug || (!assert(overlap(r, s), TRUE) || !assert(overlap(s, r), TRUE))){
		graph(space, 9);
		printf("Overlap %s\n",overlap(r, s)?"TRUE":"FALSE");
		printf("Overlap %s\n",overlap(s, r)?"TRUE":"FALSE");
	}

	r.x=0;	r.y=0; r.width=9; r.height=9;	r.created=0;
	s.x=3;	s.y=3; s.width=3; s.height=3;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	if(debug || (!assert(overlap(r, s), TRUE) || !assert(overlap(s, r), TRUE))){
		graph(space, 9);
		printf("Overlap %s\n",overlap(r, s)?"TRUE":"FALSE");
		printf("Overlap %s\n",overlap(s, r)?"TRUE":"FALSE");
	}

printf("Test FALSE overlaps\n");
	r.x=0;	r.y=0; r.width=3; r.height=9;	r.created=0;
	s.x=3;	s.y=3; s.width=6; s.height=3;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	if(debug || (!assert(overlap(r, s), FALSE) || !assert(overlap(s, r), FALSE))){
		graph(space, 9);
		printf("Overlap %s\n",overlap(r, s)?"TRUE":"FALSE");
		printf("Overlap %s\n",overlap(s, r)?"TRUE":"FALSE");
	}

	r.x=0;	r.y=0; r.width=9; r.height=3;	r.created=0;
	s.x=3;	s.y=3; s.width=3; s.height=6;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	if(debug || (!assert(overlap(r, s), FALSE) || !assert(overlap(s, r), FALSE))){
		graph(space, 9);
		printf("Overlap %s\n",overlap(r, s)?"TRUE":"FALSE");
		printf("Overlap %s\n",overlap(s, r)?"TRUE":"FALSE");
	}

	r.x=0;	r.y=0; r.width=3; r.height=9;	r.created=0;
	s.x=6;	s.y=3; s.width=3; s.height=3;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	if(debug || (!assert(overlap(r, s), FALSE) || !assert(overlap(s, r), FALSE))){
		graph(space, 9);
		printf("Overlap %s\n",overlap(r, s)?"TRUE":"FALSE");
		printf("Overlap %s\n",overlap(s, r)?"TRUE":"FALSE");
	}

	r.x=0;	r.y=0; r.width=9; r.height=3;	r.created=0;
	s.x=3;	s.y=6; s.width=3; s.height=3;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	if(debug || (!assert(overlap(r, s), FALSE) || !assert(overlap(s, r), FALSE))){
		graph(space, 9);
		printf("Overlap %s\n",overlap(r, s)?"TRUE":"FALSE");
		printf("Overlap %s\n",overlap(s, r)?"TRUE":"FALSE");
	}
}
void testremoverectangle(int debug){
	int side=15;
	rectangle s,r,space[MAXFACTORS],test[MAXFACTORS];
	int txn,testcase=0;

printf("Testing removerectangle\n");

/* Initialize space */
	initstack(space,MAXFACTORS);
	s.width=s.height=side;
	s.x=s.y=0;
	s.created=0;
	s.deleted=NOTYET;
	rpush(space, initrectangle(side,side));
	if(debug){
		printf(" Initial space\n");
		dumpstack(space);EOL;
	}
/* Now remove a rectangle at the upper left corner */
	r.width=r.height=side/2;
	r.x=r.y=0;
	r.created=0;
	r.deleted=NOTYET;
	txn=nexttransaction(space);
	removerectangle(space, r, txn);
/* Test setup - test case 0 */
	initstack(test,MAXFACTORS);
	rpush(test,initrectangle(side,side-(side/2)));
	rpush(test,initrectangle(side-(side/2),side-(side/2)));
	if(!assert(testsamerectangleset(space,test),TRUE)){
		printf(" Test case %d failed\n",testcase);
	}
	if(debug){
		dumpstack(space);
		graph(space, side);
	}

/* Initialize space */
	initstack(space,MAXFACTORS);

	s.width=24;
	s.height=20;
	s.x=15;
	s.y=8;
	s.created=11;
	s.deleted=NOTYET;
	rpush(space, s);

	s.width=15;
	s.height=8;
	s.x=0;
	s.y=8;
	s.created=11;
	s.deleted=NOTYET;
	rpush(space, s);

	if(debug)dumpstack(space);
/* Now remove a rectangle at the upper left corner */

	r.width=38;
	r.height=8;
	r.x=0;
	r.y=8;
	r.created=0;
	r.deleted=NOTYET;
	txn=nexttransaction(space);
	removerectangle(space, r, txn);
	if(debug)dumpstack(space);

/* Test setup - test case 1 */
	testcase++;
	initstack(test,MAXFACTORS);
	rpush(test,initrectangle(side,side-(side/2)));
	rpush(test,initrectangle(side-(side/2),side-(side/2)));
	if(!assert(testsamerectangleset(space,test),TRUE)){
		printf(" Test case %d failed\n",testcase);
	}


	printf("Test TRUE overlaps\n");

	r.x=0;	r.y=0; r.width=6; r.height=6;	r.created=0;
	s.x=3;	s.y=3; s.width=6; s.height=6;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	graph(space, 9);
	pop(space);
	printf("Remove ");dumprectangle(s);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, s, txn);
	graph(space, 9);

	initstack(space,MAXFACTORS);	/* Opposite test case */
	rpush(space, s);
	printf("Remove ");dumprectangle(r);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, r, txn);
	graph(space, 9);






	r.x=0;	r.y=3; r.width=6; r.height=6;	r.created=0;
	s.x=3;	s.y=0; s.width=6; s.height=6;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	graph(space, 9);
	pop(space);
	printf("Remove ");dumprectangle(s);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, s, txn);
	graph(space, 9);

	initstack(space,MAXFACTORS);	/* Opposite test case */
	rpush(space, s);
	printf("Remove ");dumprectangle(r);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, r, txn);
	graph(space, 9);

	r.x=0;	r.y=0; r.width=6; r.height=9;	r.created=0;
	s.x=3;	s.y=3; s.width=6; s.height=3;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	graph(space, 9);
	pop(space);
	printf("Remove ");dumprectangle(s);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, s, txn);
	graph(space, 9);

	initstack(space,MAXFACTORS);	/* Opposite test case */
	rpush(space, s);
	printf("Remove ");dumprectangle(r);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, r, txn);
	graph(space, 9);

	r.x=3;	r.y=0; r.width=6; r.height=9;	r.created=0;
	s.x=0;	s.y=3; s.width=6; s.height=3;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	graph(space, 9);
	pop(space);
	printf("Remove ");dumprectangle(s);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, s, txn);
	graph(space, 9);

	initstack(space,MAXFACTORS);	/* Opposite test case */
	rpush(space, s);
	printf("Remove ");dumprectangle(r);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, r, txn);
	graph(space, 9);

	r.x=0;	r.y=0; r.width=9; r.height=6;	r.created=0;
	s.x=3;	s.y=3; s.width=3; s.height=6;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	graph(space, 9);
	pop(space);
	printf("Remove ");dumprectangle(s);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, s, txn);
	graph(space, 9);

	initstack(space,MAXFACTORS);	/* Opposite test case */
	rpush(space, s);
	printf("Remove ");dumprectangle(r);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, r, txn);
	graph(space, 9);

	r.x=0;	r.y=3; r.width=9; r.height=6;	r.created=0;
	s.x=3;	s.y=0; s.width=3; s.height=6;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	graph(space, 9);
	pop(space);
	printf("Remove ");dumprectangle(s);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, s, txn);
	graph(space, 9);

	initstack(space,MAXFACTORS);	/* Opposite test case */
	rpush(space, s);
	printf("Remove ");dumprectangle(r);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, r, txn);
	graph(space, 9);

	r.x=0;	r.y=0; r.width=3; r.height=9;	r.created=0;
	s.x=0;	s.y=3; s.width=6; s.height=3;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	graph(space, 9);
	pop(space);
	printf("Remove ");dumprectangle(s);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, s, txn);
	graph(space, 9);

	initstack(space,MAXFACTORS);	/* Opposite test case */
	rpush(space, s);
	printf("Remove ");dumprectangle(r);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, r, txn);
	graph(space, 9);

	r.x=0;	r.y=0; r.width=9; r.height=3;	r.created=0;
	s.x=3;	s.y=0; s.width=3; s.height=6;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	graph(space, 9);
	pop(space);
	printf("Remove ");dumprectangle(s);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, s, txn);
	graph(space, 9);

	initstack(space,MAXFACTORS);	/* Opposite test case */
	rpush(space, s);
	printf("Remove ");dumprectangle(r);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, r, txn);
	graph(space, 9);

	r.x=0;	r.y=0; r.width=9; r.height=9;	r.created=0;
	s.x=3;	s.y=3; s.width=3; s.height=3;	s.created=0;
	initstack(space,MAXFACTORS);
	rpush(space, r);
	rpush(space, s);
	graph(space, 9);
	pop(space);
	printf("Remove ");dumprectangle(s);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, s, txn);
	graph(space, 9);

	initstack(space,MAXFACTORS);	/* Opposite test case */
	rpush(space, s);
	printf("Remove ");dumprectangle(r);printf("\n");
	txn=nexttransaction(space);
	removerectangle(space, r, txn);
	graph(space, 9);

}
void testfunctions(int side, int rectangles, int debug){
	printf("Testing\n");

	testremoverectangle(debug);
	testoverlap(debug);
	testmatch(debug);
	testdata(debug);

}

void usage(char *argv[],int side, int n){
	printf("Usage: %s [-l <side-length>] [-n <number-of-rectangles>] [-t] [-d]\n",argv[0]);
	printf("Purpose: Calculate up to N non-congruent rectangles arranged to exactly fill a square with the specified side length.\n");
	printf("Defaults: %s -l %d -n %d\n          -d enables debugging messages and -t runs unit tests.\n",argv[0],side,n);
	exit(0);
	
}
int main(int argc, char *argv[]){
	int side=15;
	int n=5;
	int budget=0;

	int debug=FALSE;
	int test=FALSE;
	int status;

	while((status=getopt(argc,argv,"dtl:n:h")) >= 0){
		switch(status){
		case 'l':
			sscanf(optarg,"%d",&side);
			break;
		case 'n':
			sscanf(optarg,"%d",&n);
			break;
		case 'd':
			debug=TRUE;
			break;
		case 't':
			test=TRUE;
			break;
		case 'h':
		default:
			usage(argv,side,n);
		}
	}

	if(test){
		testfunctions(side,n,debug);
	}
	else {
		/* Start with rectangles which would result in a score of up to 64 to begin with.
		 * The average score has been around 60 so this is a good first try. */
		/* For better scores for smaller squares (50x50), set budget to 32 initially */
		budget=64;
		if(debug)printf("Goal rectangles: %d, budget: %d\n",n,budget);
		while(solve(side,n,budget,debug) == FALSE){
			/* If we didn't find a solution, bump up the budget by 16 and try again */
			budget+=16;
			if(debug)printf("Goal rectangles: %d, budget: %d\n",n,budget);
		}
	}
}
