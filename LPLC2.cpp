/*
*File name : LPLC2.cpp
*Discription : 

    * 1. Malloc for Each Layer
        1. Better Adaption for Colias  
        2. To Avoid Stack overflow
    * 2. Visiable Computational Layers, Output Vedio
    * 3. Compressed ON/OFF Signal
    * 4. Dynamic Delay Mechanism 
    * 5. Attention Mechanism
    * 6. Data Structure : Doubly Linked List.
    * 7. Set 'Existed Attention Fields' to Supervise All AFs
    * 8. Elaborately Print Information on the Terminal.
    * 9. Structure Encapsulation :
        1. We use 3-layer nested structure:
           *The first layer Handle:
               contains basic parameters for controling Colias and Model_structure
           *The second layer Model:
               contains three kinds of information for LPLC2 model:
           *The third layer Params, Layers, Results:
               1. Parameters: all params to construct this model
               2. Layers: middle calculation information from raw vedio, 
                          representing different signal information consistent 
                          with biological anatomical structure.
               3. Results: LPLC2 calculated results for output.
        2. These structures help categorize and quickly find the required parameters or results.
        3. To simplize pointer passing progress, use:
            LPLC2struct_Params* Params = &hLPLC2->aModel.Params;
    * 10. This file combined all files in LPLC2 project, the project contains:
        1. LPLC2_model.h         ---\       
        2. coliasSense_LPLC2.h    ---\
                                        --->  LPLC2.cpp 
        3. coliasSense_LPLC2.cpp  ---/      
        4. main.cpp              ---/   
    
    
    

    
    
*Parameters Tuning:

       Name                The impact of increasing parameter values
    * tau_hp             --> receive more information from current time
    * tau_lp             --> receive more information from previous time
    * width_exc/inh/cn   --> reduce background noice, whilst also slowing program running
    * coe_Weight_E/I     --> E bigger(1->10) enhences image luminance, I bigger reduces noice.
    * remain_rec         --> causing afterimage begin from ON/OFF layer
    * coe_cn             --> get smaller normalised information(0.999->0.95) from tanh()         
    * cn_threshold       --> greatly reduce noise 
    * tau_lp             --> could adapt higher motion velocity
    * sd                 --> could adapt higher motion velocity and higher resolution
    * emd_bias           --> greatly enhence direction selectivity
    * sd_connected       --> adapt higher speed
    * sd_step            --> adapt higher resolution
    * tau_lp_dynamic[0]  --> adapt higher speed
    * T4/T5_lp           --> enhence coherence of T4/T5 layer output between two consecutive frames 
    * exp_ON/OFF         --> adjust signal weight of ON/OFF channel           
                 



*Author : Renyuan Liu
*Date : 2024 Sep, Oct.
*Tips : Anatomy-based modeling part of this program is adapted from LPLC2_C# of Dr.Fu.
*/


#include <opencv2/opencv.hpp> // only used for vedio input and output
#include <iostream>
#include <vector>
#include <stdint.h>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <fstream>
#include <algorithm>

#define USE_MULTI_ATTENTION_MECHANISM 1 // 0 or 1, with the highest priority !    [see line 2351]

#define USE_ATTENTION_MECHANISM 1 // 0 or 1
#define USE_DYNAMIC_DELAY 1 // 0 or 1



#define FRAME_BEGIN 1
#define FRAME_END 81

//#define OUTPUT_LENGTH 400 // Deprecated. maximum of frams output, = FRAME_END-FRAME_BEGIN +1
#define LPLC2_ACTIVE_LENGTH 5
#define M_PI 3.14159265358979323846



/****************** No-attention output : ******************/

// basic experiment : 
/*
#define VEDIO_PATH ".\\synthetic_vedios\\Vedio_basic_test\\test_white_recede.avi"
#define OUTPUT_VEDIO_NAME ".\\Output_Vedio\\test_white_recede.avi"
#define OUTPUT_FILE_NAME_SingleAF ".\\Output_Txt\\test_white_recede_LPLC2.txt"

#define OUTPUT_FILE_NAME_Existed ".\\Output_Txt\\1.txt"
#define OUTPUT_FILE_NAME_Existing ".\\Output_Txt\\2.txt"
#define OUTPUT_FILE_NAME_Fatal ".\\Output_Txt\\3.txt"
*/

// 4-period experiment : 
/*
#define VEDIO_PATH ".\\synthetic_vedios\\Vedio_dynamic_complex_bg\\outdoor_2_black_4_period.avi"
#define OUTPUT_VEDIO_NAME ".\\Output_Vedio\\outdoor_2_black_4_period_attention.avi"
#define OUTPUT_FILE_NAME_SingleAF ".\\Output_Txt\\outdoor_2_black_4_period_no_attention.txt"

#define OUTPUT_FILE_NAME_Existed ".\\Output_Txt\\1.txt"
#define OUTPUT_FILE_NAME_Existing ".\\Output_Txt\\2.txt"
#define OUTPUT_FILE_NAME_Fatal ".\\Output_Txt\\3.txt"
*/


/****************** Single-attention output : ******************/ 

// basic experiment : 
/*
#define VEDIO_PATH ".\\synthetic_vedios\\Vedio_basic_test\\test_grating.avi"
#define OUTPUT_VEDIO_NAME ".\\Output_Vedio\\test_grating.avi"
#define OUTPUT_FILE_NAME_SingleAF ".\\Output_Txt\\test_grating_aLPLC2.txt"

#define OUTPUT_FILE_NAME_Existed ".\\Output_Txt\\1.txt"
#define OUTPUT_FILE_NAME_Existing ".\\Output_Txt\\2.txt"
#define OUTPUT_FILE_NAME_Fatal ".\\Output_Txt\\3.txt"
*/

// 4-period experiment : 
/*
#define VEDIO_PATH ".\\synthetic_vedios\\Vedio_dynamic_complex_bg\\outdoor_2_black_4_period.avi"
#define OUTPUT_VEDIO_NAME ".\\Output_Vedio\\outdoor_2_black_4_period_attention.avi"
#define OUTPUT_FILE_NAME_SingleAF ".\\Output_Txt\\outdoor_2_black_4_period_attention.txt"

#define OUTPUT_FILE_NAME_Existed ".\\Output_Txt\\1.txt"
#define OUTPUT_FILE_NAME_Existing ".\\Output_Txt\\2.txt"
#define OUTPUT_FILE_NAME_Fatal ".\\Output_Txt\\3.txt"
*/



/****************** Multiple-attention output : ******************/ 

// basic experiment : 
/*
test_black_approach
test_black_recede
test_white_approach
test_white_recede
test_translating
test_grating
*/
/*
#define VEDIO_PATH ".\\synthetic_vedios\\Vedio_basic_test\\test_white_recede.avi"
#define OUTPUT_VEDIO_NAME ".\\Output_Vedio\\test_white_recede.avi"

#define OUTPUT_FILE_NAME_SingleAF ".\\Output_Txt\\4.txt"
#define OUTPUT_FILE_NAME_Existed ".\\Output_Txt\\5.txt"
#define OUTPUT_FILE_NAME_Existing ".\\Output_Txt\\6.txt.txt"
#define OUTPUT_FILE_NAME_Fatal ".\\Output_Txt\\test_white_recede_mLPLC2.txt"
*/

// 4-period experiment : 

/*
#define VEDIO_PATH ".\\synthetic_vedios\\Vedio_dynamic_complex_bg\\outdoor_2_black_4_period.avi"
#define OUTPUT_VEDIO_NAME ".\\Output_Vedio\\outdoor_2_black_4_period.avi"

#define OUTPUT_FILE_NAME_SingleAF ".\\Output_Txt\\4.txt"
#define OUTPUT_FILE_NAME_Existed ".\\Output_Txt\\outdoor_2_black_4_period_Existed.txt"
#define OUTPUT_FILE_NAME_Existing ".\\Output_Txt\\outdoor_2_black_4_period_Existing.txt"
#define OUTPUT_FILE_NAME_Fatal ".\\Output_Txt\\outdoor_2_black_4_period_Fatal.txt"
*/

// 6-objects experiment : 

/*
#define VEDIO_PATH ".\\synthetic_vedios\\Vedio_dynamic_complex_bg\\outdoor_3_black_expand_multi_6.avi"
#define OUTPUT_VEDIO_NAME ".\\Output_Vedio\\outdoor_3_black_expand_multi_6.avi"

#define OUTPUT_FILE_NAME_SingleAF ".\\Output_Txt\\4.txt"
#define OUTPUT_FILE_NAME_Existed ".\\Output_Txt\\outdoor_3_black_expand_multi_6_Existed.txt"
#define OUTPUT_FILE_NAME_Existing ".\\Output_Txt\\outdoor_3_black_expand_multi_6_Existing.txt"
#define OUTPUT_FILE_NAME_Fatal ".\\Output_Txt\\outdoor_3_black_expand_multi_6_Fatal.txt"
*/

// UAV_outdoor : 

#define VEDIO_PATH "C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_dynamic_complex_bg\\standared_car_attack.avi"
#define OUTPUT_VEDIO_NAME ".\\Output_Vedio\\standared_car_attack.avi"

#define OUTPUT_FILE_NAME_SingleAF ".\\Output_Txt\\4.txt"
#define OUTPUT_FILE_NAME_Existed ".\\Output_Txt\\standared_car_attack_Existed.txt"
#define OUTPUT_FILE_NAME_Existing ".\\Output_Txt\\standared_car_attack_Existing.txt"
#define OUTPUT_FILE_NAME_Fatal ".\\Output_Txt\\standared_car_attack_Fatal.txt"



int Field_Number = 0; // Attention Field : Who am I ? 

int Width = 0;
int Height = 0;
int Total_Frame = 0;
double FPS = 0;




#pragma region Structures defination

typedef struct Attention{

    struct Attention* prev;
    struct Attention* next ;

    // Centroid of Current Attention
    // 0->Ordinate, 1->Abscissa
    int Attention_Coordinate[2];


    // Four Quadrants Response in Receptive Field
    double RF_Quadrant_Output[4];


    // Current Response in Receptive Field
    double RF_Current_Output[LPLC2_ACTIVE_LENGTH];
    // Recent Response in this field, to judge wether to abandon this attention field (compare with T_{AF})
    double RF_Recent_Output; // latest 5 frames.

    // Response of LPLC2 Visual Projection Neuron
    //double RF_Output[OUTPUT_LENGTH];
    double RF_Output[FRAME_END-FRAME_BEGIN +1];
    

    // Who am I ?
    int Number ;
    // Existing[0] : debut time, [1] : disappear time.
    int Existing[2] ;

}Attention; // level 5

typedef struct FieldSet{
    Attention* front;
    Attention* rear;
    int size;
}FieldSet; // level 4


typedef struct
{
    // Threshold for collision
    double T_collision ;

    // Judge wether abondon current attention field or not : 
    // observation time for recent response
    int d ;
    // Threshold for recent response 
    double T_af ;

    // Tereshold for Activate LPLC2
    double T_lplc2 ;

    // Magnify Coeeficient
    double Magnify ;

    // Judge whether to establish a new attention field or not by evaluating distance to current existed fields
    int Judge_Field_size ;

    // Receptive Field size
    int RF_size;

    // sampling distance
    int sd; 
    // sampling daitance(step length) in Dynamic Delay Mechanism
    int sd_step;
    // connected neuron number
    int sd_connected;

    // whether blocking ON/OFF channel
    bool blockON;
    bool blockOFF;
    // whether blocking contrast vision 
    bool blockContrast;
    // whether blocking LPTC/LPLC2
    bool blockLPTC; 
    bool blockLPLC2;

    // clip point in half-wave rectifying
    double clip;
    // DC component in ON/OFF half-wave rectifying
    double remain_rec;

    // time constant of Retina layer: high-pass (ms)
    int tau_hp;
    // delay coefficient, calculated by tau
    double delay_hp;
    // time constant of signal correlation in Medulla layer: low-pass (ms)
    int tau_lp;
    // delay coefficient
    double delay_lp;
    // time constant of T4 cells: low-pass (ms)
    int T4_lp;
    // delay coefficient
    double delay_T4;
    // time constant of T5 cells: low-pass (ms)
    int T5_lp;
    // delay coefficient
    double delay_T5;
    // Dynamic Delay Mechanism :  
    int tau_lp_dynamic[100]; // max number of connected neurons is 50
    double delay_lp_dynamic[100];

    double std_exc;
    // standard deviation in inhibitory field
    double std_inh;
    // standard deviation in contrast normalisation field
    double std_cn;


    // width of convolution kernel in contrast normalisation field
    int width_cn;
    // coefficient in contrast normalisasion field
    double coe_cn;   
    // threshold in contrast normalisasion field for compressed signal within 0 ~ 1
    double cn_threshold;
    // width of convolution kernel in excitation field
    int width_exc;
    // width of convolution kernel in inhibition field
    int width_inh;

    // coefficient of parallel motion pathway in summation
    double coe_motion;
    // coefficient of parallel contrast pathway in sumation
    double coe_contrast;

    // standard deviation (horizontal) in radial spatial bias dsitribution
    double std_w;
    // standard deviation (vertical) in radial spatial bias dsitribution
    double std_h;
    // scale parameter (horizontal) in making gaussian gaussian dsitribution
    double scale_w;
    // scale parameter (vertical) in making gaussian gaussian dsitribution
    double scale_h;


    // interval between two discrete frames
    double interval;
    // bias in two-arm motion signal correlation
    double Mbias;
    double emd_bias;
    // time window in frames for averaging
    //int time_length;


    // Weight in Excitaion, Inhibition, Summation layer
    double coe_Weight_E;
    double coe_Weight_I;

    // exponent of T4 neuron response
    double exp_ON;
    // exponent of T5 neuron response
    double exp_OFF;
    // standard deviation in excitatory field



}LPLC2struct_Params; // level 3


typedef struct
{
    // std::int8_t Diff_Img[2][Height][Width];  // ------>> STACK OVERFLOW !!!!!


    // Diff_Img[2][Height][Width]

    // std::int8_t*** Diff_Img;
    std::int16_t*** Diff_Img;
    //double*** Diff_Img;

    // gaussian blur layer[Height][Width]
    //double** GB;
    double** Summation;
    // convolution kernel in gaussian blur
    double** Kernel_GB_Exc;
    double** Kernel_GB_Inh;


    // ON/OFF channels [2][Height][Width]
    double*** ON;
    double*** OFF;

    // convolution keinel in contrast normalization
    double** Kernel_Contrast;
    // ON/OFF compressed signal
    double*** ON_Compressed;
    double*** OFF_Compressed;

    // contrast vision in ON/OFF channels
    double*** ON_Contrast;
    double*** OFF_Contrast;
    // ON/OFF channels delayed signal
    double*** ON_Delay;
    double*** OFF_Delay;


    // T4 neurons in Medulla
    // Dimension: 2 * width * height * 4 (cardinal directions)
    // 0->rightward, 1->leftward, 2->downward, 3->upward
    double**** T4s;
    // T5 neurons in Lobula
    // Dimension: 2 * width * height * 4 (cardinal directions)
    // 0->rightward, 1->leftward, 2->downward, 3->upward
    double**** T5s;
    // direction indicator matrix of T4/T5 neurons
    double** T4_direction;
    double** T5_direction;
    // motion magnitude matrix of T4/T5 neurons
    double** T4_mag;
    double** T5_mag;
    // local motion signal integrated from T4/T5 cells
    // Dimension: width* height * 4 (cardinal directions)
    double*** LM;


    // LPTC : 
    // LPTCs motion magnitude
    double** LPTCs_mag;
    // LPTCs motion direction
    double** LPTCs_direction;

    // LPTCs in Lobula Plate
    // Dimension 4 * 1 vector
    // 0->rightward, 1->leftward, 2->downward, 3->upward
    double* LPTCs;


    // Lobula Plate intrinsic interneurons inhibiting stratified LPTCs
    // Dimension 4 * 1 vector
    // 0->leftward, 1->rightward, 2->upward, 3->downward
    double* LPis;



    // LPLC2 :
    // LPLC2s motion magnitude
    double** LPLC2s_mag;
    // LPLC2s motion direction
    double** LPLC2s_direction;

    // Spatial bias distribution on radial outward motion
    double** Radial_Bias;


    // Motion Signal Map (RGB) of T4/T5/LPTCs/LPLC2s neurons
    std::uint8_t*** T4_msm;
    std::uint8_t*** T5_msm;
    std::uint8_t*** LPTCs_msm;
    std::uint8_t*** LPLC2_msm;


}LPLC2struct_Layers; // level 3


typedef struct
{
    // system of horizonal sensitive
    double SysHS;
    // system of vertical sensitive
    double SysVS;

    // max motion of T4/T5 neurons
    double T4_maxMag;
    double T5_maxMag;
    double MaxMag;
    int MaxMag_y;
    int MaxMag_x;

    // max motion beyond current existed attention field 
    double BEYOND;
    int BEYOND_y;
    int BEYOND_x;



    // regional response of whole population of LPLC2 visual projection neurons
    // Dimension 4 * 1 vector
    // 0->left_up, 1->right_up, 2->left_bottom, 3->right_bottom
    double LPLC2_Quadrant_Output[4];
    // Centroid of view
    // Dimension 2 * 1 vector
    // 0->ordinate, 1->abscissa
    int Attention_Coordinate[2];

    // visual projection neurons LPLC2 response
    double LPLC2[LPLC2_ACTIVE_LENGTH];
    // averaged output response of LPLC2 visual projection neurons
    //double LPLC2_Output[OUTPUT_LENGTH]; 
    double LPLC2_Output[FRAME_END-FRAME_BEGIN +1]; 



    // Multi Attention Mechanism 
    FieldSet* ExistingAFs ; // Existing
    FieldSet* ExistedAFs ; // Existed
    FieldSet* FatalAFs ; // AFs that correctly recegnize impending objects.

}LPLC2struct_Results; // level 3



typedef struct
{
    LPLC2struct_Params Params;
    LPLC2struct_Layers Layers;
    LPLC2struct_Results Results;

}LPLC2Type; // level 2


typedef struct {
    // LPLC2Type* aModel; 
    LPLC2Type aModel; // level 2 

    std::uint32_t processCount;
    std::uint32_t processCountLast;
    std::uint32_t* hFrameCount;
    std::uint8_t Enable;
    std::uint8_t status;
    std::uint8_t processRate;
    std::uint8_t currentImage; // t-2, t-1, t
    bool currentDiffImage; // t-1, t
    bool previousDiffImage; // t, t-1
    // std::uint8_t AGC_enable_period;

}LPLC2_pControlTypedef; // level 1

// declaration and allocation of structures : in main founction
#pragma endregion Structure_End



#pragma region Founctions defination

// Attention Field Operation :
FieldSet* createFieldSet() 
{
    FieldSet* fields = (FieldSet*)malloc(sizeof(FieldSet ) ) ;
    fields->front = NULL;
    fields->rear = NULL;
    fields->size = 0; 
    return fields;
}

// insert at tail : 
void enFields(FieldSet* fields, Attention* newAttention) 
{
    if (fields->rear == NULL) {
        fields->front = fields->rear = newAttention;
        newAttention->next = newAttention->prev = NULL;
    } else {
        fields->rear->next = newAttention;
        newAttention->prev = fields->rear; 
        fields->rear = newAttention;
        newAttention->next = NULL;
    }

    fields->size++; 
    //printf("\n\nNew Attention Field Enfields !!!!!!!!!!!!!!!!!! \n");
}
// delete at front : 
Attention* deFields(FieldSet* fields) 
{
    if (fields->front == NULL) {
        return NULL; 
    }
    Attention* temp = fields->front;
    fields->front = fields->front->next;

    if (fields->front != NULL) {
        fields->front->prev = NULL; 
    } else {
        fields->rear = NULL; 
    }

    fields->size--; 
    printf("\n\nAttention Field Defields !!!!!!!!!!!!!!!!!! \n");
    return temp;
}
// delete at radom position : 

void void_removeField(FieldSet* fields, Attention* attention, int time) 
{
// record deField time : 
attention->Existing[1] = time ;

if (attention->prev != NULL) {
attention->prev->next = attention->next;
} else {
fields->front = attention->next ; 
}

if (attention->next != NULL) {
attention->next->prev = attention->prev;
} else {
fields->rear = attention->prev ; 
}
printf("\n\nAttention Node [%d] Removed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n", attention->Number );

// if free node here in existing AFs, node in existed AFs also be freed since they are the identical node.
// free(attention) ; 
fields->size--;

return ;
}

Attention* removeField(FieldSet* fields, Attention* attention, int time) 
{
    // record deField time : 
    attention->Existing[1] = time ;

    if (attention->prev != NULL) {
        attention->prev->next = attention->next;
    } else {
        fields->front = attention->next ; 
    }

    if (attention->next != NULL) {
        attention->next->prev = attention->prev;
    } else {
        fields->rear = attention->prev ; 
    }
    printf("\nAttention Node [%d] have been Removed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ", attention->Number );

    // if free node here in existing AFs, node in existed AFs also be freed since they are the identical node.
    // free(attention);
    fields->size--;

    return attention ;
}

bool isEmpty(FieldSet* fields) 
{
    return fields->front == NULL;
}

void freeFieldSet(FieldSet* fields) 
{
    if (fields == NULL) {
        printf("\nFields is NULL, no need to free !!! \n");
        return;
    }

    Attention* current = fields->front;
    Attention* next;

    while (current != NULL) 
    {
        next = current->next;

        printf("\nTry to Free Current Field [%d] ... , ", current->Number );
        if(next != NULL){
            printf("Next is Field [%d] ",next->Number );
        }else{
            printf("Next is NULL ") ;
        }
        free(current);
        current = next;

        // Debug : 
        /*
        if (next == NULL )
        {
        current = NULL ; 
        printf("\n\nCurrent is Set to NULL. \n") ;
        break; 
        }else{
        current = next;
        printf("Current is Set to Field[%d]. \n", next->Number ) ;
        }
        */
    }
    free(fields);
}

void traverseFieldSet(Attention* head, LPLC2_pControlTypedef* hLPLC2)  
{
    for (Attention* current = head; current != nullptr; current = current->next ) 
    {

        printf("\n____________________________________________________________________________\nField [%d]",current->Number );

        printf("\nCoordinates: (%d, %d), (In:%d, Out:%d) \n", current->Attention_Coordinate[0], current->Attention_Coordinate[1], current->Existing[0], current->Existing[1] );

        for (int i = 0; i < 4; i++) {
            //printf("RF Quadrant Output [%d]: %f\n", i, current->RF_Quadrant_Output[i]);
        }
        //printf("\n");
        // Field->RF_Current_Output[*hLPLC2->hFrameCount % LPLC2_ACTIVE_LENGTH]
        for (int i = 0; i < LPLC2_ACTIVE_LENGTH; i++) {
            //printf("RF Current Output [%d]: %f\n", i, current->RF_Current_Output[*hLPLC2->hFrameCount % LPLC2_ACTIVE_LENGTH]/*  current->RF_Current_Output[i]  */);

            //printf("RF Current Output [%d]: %f\n", i, current->RF_Current_Output[i] );
        }

        int n = *hLPLC2->hFrameCount ;
        printf("Output in Frame[%d]  : %f, ", n, current->RF_Output[n] );
        printf("Sum Recent %d Output : %f\n", hLPLC2->aModel.Params.d, current->RF_Recent_Output );

    }
}

void filtFatalAFsFromExisted(LPLC2_pControlTypedef* hLPLC2, FieldSet* AllFields, FieldSet* FatalFields)
{
    printf("\n\nFilting Fatal AFs From All Existed AFs : ") ;
    for (Attention* current = AllFields->front; current != nullptr; current = current->next ) 
    {
        /*
        // way 1 : This way destroies the Existed AFs.
        double max_mp = *std::max_element(current->RF_Output, current->RF_Output + 120) ;
        printf("\nscanning field [%d] : Coordinates[%d,%d] max_output : %f ", current->Number, current->Attention_Coordinate[0],current->Attention_Coordinate[1], max_mp ) ;
        // Add new AF to AFs : 
        if(max_mp >= hLPLC2->aModel.Params.T_collision)
        {
            Attention* next = current->next ;
            enFields(FatalFields, current) ;
            current = next ;
        }
        */


        // way 2 : This way will conserve the Existed AFs.
        // Create new AF to 'Fatal AFs'
        Attention* NEW3 = (Attention* )malloc(sizeof(Attention) ) ;
        NEW3->next = NULL ;
        NEW3->Attention_Coordinate[0] = current->Attention_Coordinate[0] ;
        NEW3->Attention_Coordinate[1] = current->Attention_Coordinate[1] ;
        NEW3->Existing[0] = current->Existing[0] ;
        NEW3->Existing[1] = current->Existing[1] ;
        for(int i = 0; i < FRAME_END-FRAME_BEGIN +1; i ++ )
        {
            NEW3->RF_Output[i] = current->RF_Output[i] ;
        }
        NEW3->RF_Recent_Output = 0 ;
        // Who am I ?
        NEW3->Number = current->Number ;

        // Add new AF to AFs : 
        double max_mp = *std::max_element(NEW3->RF_Output, NEW3->RF_Output + FRAME_END-FRAME_BEGIN +1) ;
        printf("\nscanning field [%d] : Coordinates[%d,%d] max_output : %f ", NEW3->Number, NEW3->Attention_Coordinate[0],NEW3->Attention_Coordinate[1], max_mp ) ;
        if(max_mp >= hLPLC2->aModel.Params.T_collision)
        {
            enFields(FatalFields, NEW3) ;
            printf("\nNew AF [%d] enField to Fatal AFs. ", NEW3->Number ) ;
        }
        
    }

}

// Output to .txt file : 
// when use single-attention-mechanism, use this : 
void writeToText(LPLC2_pControlTypedef hLPLC2)
{
    if (std::remove(OUTPUT_FILE_NAME_SingleAF) == 0) {
        std::cout << "To prevent rewrite from the last line of the previous file, we've deleted: " << OUTPUT_FILE_NAME_SingleAF << std::endl;
    } else {
        std::cout << "File not exist, no need to delete." << OUTPUT_FILE_NAME_SingleAF << std::endl;
    }

    std::ofstream outfile(OUTPUT_FILE_NAME_SingleAF);  

    if (!outfile.is_open()) {
        std::cerr << "Open .txt File Failed ! " << std::endl;
    }

    for (int i = 0; i < FRAME_END-FRAME_BEGIN +1; i ++ ) 
    {
        outfile << hLPLC2.aModel.Results.LPLC2_Output[FRAME_BEGIN + i] << std::endl;
    }

    outfile.close();

    printf("\n\nLPLC2 Processed Information Output to %s File Over\n", OUTPUT_FILE_NAME_SingleAF );

}

// output current existing AFs, use this : (there should be no Existing AFs when vedio is over.)
void writeToText_Existing_AFs(FieldSet* ExistedAFs)
{
    if (std::remove(OUTPUT_FILE_NAME_Existing) == 0) {
        std::cout << "\nTo prevent rewrite from the last line of the previous file, we've deleted: " << OUTPUT_FILE_NAME_Existing << std::endl;
    } else {
        std::cout << "\nFile not exist, no need to delete." << OUTPUT_FILE_NAME_Existing << std::endl;
    }

    // 使用 std::ios::app 以追加模式打开文件
    std::ofstream outfile(OUTPUT_FILE_NAME_Existing, std::ios::app);  

    if (!outfile.is_open()) {
        std::cerr << "Open .txt File Failed !!" << std::endl;
        return;
    }

    for (Attention* current = ExistedAFs->front; current != nullptr; current = current->next) 
    {

        outfile << current->Number << " "
            << current->Attention_Coordinate[0] << " "
            << current->Attention_Coordinate[1] << " "
            << current->Existing[0] << " "  // debut 
            << current->Existing[1] << " "; // out

        for (int i = 0; i < FRAME_END-FRAME_BEGIN +1; i++) {
            outfile << current->RF_Output[i] << " ";
            //printf("*****%f",current->RF_Output[i]);
        }

        outfile << std::endl;
    }

    outfile.close();

    printf("\nCurrent Existing Fields of LPLC2 Processed Information Output to %s File Over", OUTPUT_FILE_NAME_Existing );

}

// output all existed AFs, use this : 
void writeToText_Existed_AFs(FieldSet* ExistedAFs)
{
    if (std::remove(OUTPUT_FILE_NAME_Existed) == 0) {
        std::cout << "\nTo prevent rewrite from the last line of the previous file, we've deleted: " << OUTPUT_FILE_NAME_Existed << std::endl;
    } else {
        std::cout << "\nFile not exist, no need to delete." << OUTPUT_FILE_NAME_Existed << std::endl;
    }

    // 使用 std::ios::app 以追加模式打开文件
    std::ofstream outfile(OUTPUT_FILE_NAME_Existed, std::ios::app);  

    if (!outfile.is_open()) {
        std::cerr << "Open .txt File Failed !!" << std::endl;
        return;
    }

    for (Attention* current = ExistedAFs->front; current != nullptr; current = current->next) 
    {

        outfile << current->Number << " "
            << current->Attention_Coordinate[0] << " "
            << current->Attention_Coordinate[1] << " "
            << current->Existing[0] << " "  // debut 
            << current->Existing[1] << " "; // out

        for (int i = 0; i < FRAME_END-FRAME_BEGIN +1; i++) {
            outfile << current->RF_Output[i] << " ";
            //printf("*****%f",current->RF_Output[i]);
        }

        outfile << std::endl;
    }

    outfile.close();

    printf("\nAll Existed Fields of LPLC2 Processed Information Output to %s File Over", OUTPUT_FILE_NAME_Existed );

}

// output AFs that correctly recognize impending objects, use this : 
void writeToText_Fatal_AFs(FieldSet* FatalAFs)
{
    if (std::remove(OUTPUT_FILE_NAME_Fatal) == 0) {
        std::cout << "\nTo prevent rewrite from the last line of the previous file, we've deleted: " << OUTPUT_FILE_NAME_Fatal << std::endl;
    } else {
        std::cout << "\nFile not exist, no need to delete." << OUTPUT_FILE_NAME_Fatal << std::endl;
    }

    // 使用 std::ios::app 以追加模式打开文件
    std::ofstream outfile(OUTPUT_FILE_NAME_Fatal, std::ios::app);  

    if (!outfile.is_open()) {
        std::cerr << "Open .txt File Failed !!" << std::endl;
        return;
    }

    for (Attention* current = FatalAFs->front; current != nullptr; current = current->next) 
    {

        outfile << current->Number << " "
            << current->Attention_Coordinate[0] << " " // y, from top to bottom
            << current->Attention_Coordinate[1] << " " // x, from left to right
            << current->Existing[0] << " "  // debut 
            << current->Existing[1] << " "; // out

        for (int i = 0; i < FRAME_END-FRAME_BEGIN +1; i++) {
            outfile << current->RF_Output[i] << " ";
            //printf("*****%f",current->RF_Output[i]);
        }

        outfile << std::endl;
    }

    outfile.close();

    printf("\nFatal Fields of LPLC2 Processed Information Output to %s File Over", OUTPUT_FILE_NAME_Fatal );

}



// Memory Space Allocation : 
void Allocate2DLayer(std::int8_t*** Layer, int height, int width) 
{
    // [height]
    *Layer = (std::int8_t**)malloc(height * sizeof(std::int8_t*));
    if (*Layer == NULL) {
        std::cerr << "Memory allocation failed for Layer!" << std::endl;
        exit(1);
    }

    // [width] 
    for (int i = 0; i < height; ++i) {
        (*Layer)[i] = (std::int8_t*)malloc(width * sizeof(std::int8_t));
        if ((*Layer)[i] == NULL) {
            std::cerr << "Memory allocation failed for Layer[" << i << "]!" << std::endl;
            exit(1);
        }

        // memset
        std::memset((*Layer)[i], 0, width * sizeof(std::int8_t));
    }
}

void Allocate2DLayer(double*** Layer, int height, int width)
{
    // [height]
    *Layer = (double**)malloc(height * sizeof(double*));
    if (*Layer == NULL) {
        std::cerr << "Memory allocation failed for Layer!" << std::endl;
        exit(1);
    }

    // [width] 
    for (int i = 0; i < height; ++i) {
        (*Layer)[i] = (double*)malloc(width * sizeof(double));
        if ((*Layer)[i] == NULL) {
            std::cerr << "Memory allocation failed for Layer[" << i << "]!" << std::endl;
            exit(1);
        }

        // memset
        std::memset((*Layer)[i], 0, width * sizeof(double));


        // printf("row %d allocate successfully,total : %d rows \n", i, height);
    }
}

void Allocate3DLayer(std::int8_t**** Layer, int depth, int height, int width) 
{
    // [depth]
    *Layer = (std::int8_t***)malloc(depth * sizeof(std::int8_t**));
    if (*Layer == NULL) {
        std::cerr << "Memory allocation failed for Diff_Img!" << std::endl;
        exit(1);
    }

    // [height] 
    for (int i = 0; i < depth; ++i) {
        (*Layer)[i] = (std::int8_t**)malloc(height * sizeof(std::int8_t*));
        if ((*Layer)[i] == NULL) {
            std::cerr << "Memory allocation failed for Layer[" << i << "]!" << std::endl;
            exit(1);
        }

        //  [width] 
        for (int j = 0; j < height; ++j) {
            (*Layer)[i][j] = (std::int8_t*)malloc(width * sizeof(std::int8_t));
            if ((*Layer)[i][j] == NULL) {
                std::cerr << "Memory allocation failed for Layer[" << i << "][" << j << "]!" << std::endl;
                exit(1);
            }

            // memset
            std::memset((*Layer)[i][j], 0, width * sizeof(std::int8_t));
        }
    }
}

void Allocate3DLayer(std::int16_t**** Layer, int depth, int height, int width)
{
    // [depth]
    *Layer = (std::int16_t***)malloc(depth * sizeof(std::int16_t**));
    if (*Layer == NULL) {
        std::cerr << "Memory allocation failed for Diff_Img!" << std::endl;
        exit(1);
    }

    // [height] 
    for (int i = 0; i < depth; ++i) {
        (*Layer)[i] = (std::int16_t**)malloc(height * sizeof(std::int16_t*));
        if ((*Layer)[i] == NULL) {
            std::cerr << "Memory allocation failed for Layer[" << i << "]!" << std::endl;
            exit(1);
        }

        //  [width] 
        for (int j = 0; j < height; ++j) {
            (*Layer)[i][j] = (std::int16_t*)malloc(width * sizeof(std::int16_t));
            if ((*Layer)[i][j] == NULL) {
                std::cerr << "Memory allocation failed for Layer[" << i << "][" << j << "]!" << std::endl;
                exit(1);
            }

            // memset
            std::memset((*Layer)[i][j], 0, width * sizeof(std::int16_t));
        }
    }
}

void Allocate3DLayer(double**** Layer, int depth, int height, int width)
{
    // [depth]
    *Layer = (double***)malloc(depth * sizeof(double**));
    if (*Layer == NULL) {
        std::cerr << "Memory allocation failed for Layer!" << std::endl;
        exit(1);
    }

    // [height] 
    for (int i = 0; i < depth; ++i) {
        (*Layer)[i] = (double**)malloc(height * sizeof(double*));
        if ((*Layer)[i] == NULL) {
            std::cerr << "Memory allocation failed for Layer[" << i << "]!" << std::endl;
            exit(1);
        }

        //  [width] 
        for (int j = 0; j < height; ++j) {
            (*Layer)[i][j] = (double*)malloc(width * sizeof(double));
            if ((*Layer)[i][j] == NULL) {
                std::cerr << "Memory allocation failed for Layer[" << i << "][" << j << "]!" << std::endl;
                exit(1);
            }

            // memset 
            std::memset((*Layer)[i][j], 0, width * sizeof(double));
        }
    }
}

void Allocate4DLayer(double***** Layer, int time, int height, int width, int direction)
{
    // [time]
    *Layer = (double****)malloc(time * sizeof(double***));
    if (*Layer == NULL) {
        std::cerr << "Memory allocation failed for Layer!" << std::endl;
        exit(1);
    }

    // [height]
    for (int t = 0; t < time; ++t) {
        (*Layer)[t] = (double***)malloc(height * sizeof(double**));
        if ((*Layer)[t] == NULL) {
            std::cerr << "Memory allocation failed for Layer[" << t << "]!" << std::endl;
            exit(1);
        }

        // [width]
        for (int i = 0; i < height ; ++i) {
            (*Layer)[t][i] = (double**)malloc(width * sizeof(double*));
            if ((*Layer)[t][i] == NULL) {
                std::cerr << "Memory allocation failed for Layer[" << t << "][" << i << "]!" << std::endl;
                exit(1);
            }

            // [direction]
            for (int j = 0; j < width; ++j) {
                (*Layer)[t][i][j] = (double*)malloc(direction * sizeof(double));
                if ((*Layer)[t][i][j] == NULL) {
                    std::cerr << "Memory allocation failed for Layer[" << t << "][" << i << "][" << j << "]!" << std::endl;
                    exit(1);
                }

                // memset
                std::memset((*Layer)[t][i][j], 0, direction * sizeof(double));
            }
        }
    }
}


// Free Memory Space : 
void Free2DLayer(std::int8_t*** Layer, int height)
{
    if (*Layer == nullptr) return; 

    for (int i = 0; i < height; ++i) {
        free((*Layer)[i]);
    }

    free(*Layer);

    *Layer = nullptr;
}

void Free2DLayer(std::int16_t*** Layer, int height)
{
    if (*Layer == nullptr) return;

    for (int i = 0; i < height; ++i) {
        free((*Layer)[i]);
    }

    free(*Layer);

    *Layer = nullptr;
}

void Free2DLayer(double*** Layer, int height)
{
    if (*Layer == nullptr) return;

    for (int i = 0; i < height; ++i) {
        free((*Layer)[i]);
    }

    free(*Layer);

    *Layer = nullptr;
}

void Free3DLayer(std::int8_t**** Layer, int depth, int height)
{
    if (*Layer == nullptr) return;

    for (int i = 0; i < depth; ++i) {
        for (int j = 0; j < height; ++j) {
            free((*Layer)[i][j]);
        }
        free((*Layer)[i]);
    }

    free(*Layer);

    *Layer = nullptr;
}

void Free3DLayer(std::int16_t**** Layer, int depth, int height)
{
    if (*Layer == nullptr) return;

    for (int i = 0; i < depth; ++i) {
        for (int j = 0; j < height; ++j) {
            free((*Layer)[i][j]);
        }
        free((*Layer)[i]);
    }

    free(*Layer);

    *Layer = nullptr;
}

void Free3DLayer(double**** Layer, int depth, int height)
{
    if (*Layer == nullptr) return;

    for (int i = 0; i < depth; ++i) {
        for (int j = 0; j < height; ++j) {
            free((*Layer)[i][j]);
        }
        free((*Layer)[i]);
    }

    free(*Layer);

    *Layer = nullptr;
}

void Free4DLayer(double***** Layer, int time, int height, int width)
{
    if (*Layer == nullptr) return;

    for (int t = 0; t < time; ++t) {
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                free((*Layer)[t][i][j]);  // [direction]
            }
            free((*Layer)[t][i]);  // [width]
        }
        free((*Layer)[t]);  // [height]
    }

    free(*Layer);  // [time]

    *Layer = nullptr;
}


// Signal Operation : 
// low-pass for single pixel :
double Lowpass(double cur_in, double pre_out, double alpha)
{
    return alpha * cur_in + (1 - alpha) * pre_out;
}

// half-wave rectifying in terms of onset response
double Halfwave_ON(double cur_in, double pre_out, double clip, double coe_delay)
{
    if (cur_in >= clip)
        return cur_in + coe_delay * pre_out;
    else
        return coe_delay * pre_out;
}

// half-wave rectifying in terms of offset response
double Halfwave_OFF(double cur_in, double pre_out, double clip, double dc)
{
    if (cur_in < clip)
        return abs(cur_in) + dc * pre_out;
    else
        return dc * pre_out;
}

// Rectified Linear Unit 
double ReLU(double value)
{
    if (value < 0)
        return 0;
    else
        return value;
}
// Threshold ReLU
double ReLU(double value, double Threshold)
{
    if (value < Threshold)
        return 0;
    else
        return value;
}

// Leaky Rectified Linear Unit(Leaky - ReLU)
double Leaky_ReLU(double value)
{
    if (value >= 0)
        return value;
    else
        return 0.01 * value;
}

// Sigmoid Activation
double Sigmoid(double value, double coe)
{
    return pow(1 + exp(-value * pow(coe, -1 ) ), -1);
}

void ThresholdIt(double* value, double threshold)
{
    if(*value < threshold){
        *value = 0 ;
    }
}

// EMD (HRC) : 
/*
cur_p: current position signal
cur_p_d: currrent position + distance signal
pre_p: previous position signal
pre_p_d: previous position + distance signal
bias: local bias
&: cur, #: pre, -----: distance
*/

// Converging triple correlation
// 
//     &                   &  --->leftward
//     #-----#   -   #-----# 
double Cov_TripleCorrelation(double cur_p, double cur_p_d, double pre_p, double pre_p_d)
{                                                                
    return cur_p_d * pre_p_d * pre_p  - cur_p * pre_p * pre_p_d; 
}                                                                
// Converging triple correlation with bias on opposing motion
double Cov_TripleCorrelation_Bias(double cur_p, double cur_p_d, double pre_p, double pre_p_d, double bias)
{
    //double emd = bias * (pre_p * pre_p_d * cur_p_d - pre_p * cur_p * pre_p_d);
    double emd =  (pre_p * pre_p_d * cur_p_d - bias * pre_p * cur_p * pre_p_d);
    if (emd < 0 || emd > 1) // in case overflow max_pixel_value(255) of output image, which causes sharp luminance increasement.
        return 0;
    else
        return emd;
}


// Diverging tripe correlation with bias on opposing motion (on pixel)
// 
//     &-----&       &-----&
//           #   -   #       --->leftward
double Div_TripleCorrelation(double cur_p, double cur_p_d, double pre_p, double pre_p_d)
{                                                                  
    return pre_p * cur_p * cur_p_d - cur_p * pre_p_d * cur_p_d;    
}                                                                  
// Diverging tripe correlation with bias on opposing motion (on pixel)
double Div_TripleCorrelation_Bias(double cur_p, double cur_p_d, double pre_p, double pre_p_d, double bias)
{                                                                         
    double emd = bias * (pre_p * cur_p * cur_p_d - cur_p * pre_p_d * cur_p_d);  
    if (emd < 0 || emd > 1)// in case overflow max_pixel_value(255) of output image, which causes sharp luminance increasement.
        return 0;
    else
        return emd;
}                 


// HR correlation
//
//     &                   &     --->leftward
//      -----#   -   #-----     
double HRCorrelation_Bias(double cur_p, double cur_p_d, double pre_p, double pre_p_d, double bias)
{
    double emd = pre_p * cur_p_d - bias * cur_p * pre_p_d;
    if (emd < 0 || emd > 1)// in case overflow max_pixel_value(255) of output image, which causes sharp luminance increasement.
        return 0;
    else
        return emd;
}


// Polarity channels summation each with fractional order
double ON_OFF_ChannelSummation(double ON, double OFF, double on_exp, double off_exp)
{
    return pow(ON, on_exp) + pow(OFF, off_exp);
}


// Convolution : 
void MakeGaussianKernel(double sigma, int gauss_width, double*** kernel)
{
    printf("\n\n\nKERNEL :\n");
    int center = gauss_width / 2;
    double sum = 0;

    for (int i = 0; i < gauss_width; i++)
    {
        for (int j = 0; j < gauss_width; j++)
        {
            // distance
            double x = i - center;
            double y = j - center;

            // core
            (*kernel)[i][j] = (1 / (2 * M_PI * sigma * sigma)) * exp(-(x * x + y * y) / (2 * sigma * sigma));

            // sum 3 x 3
            sum += (*kernel)[i][j];
        }
    }

    // normalisation :
    for (int i = 0; i < gauss_width; i++)
    {
        printf("\n\n\nrow %d:   ", i);
        for (int j = 0; j < gauss_width; j++)
        {
            (*kernel)[i][j] /= sum;
            printf("[%f] ", (*kernel)[i][j]);
        }
    }

}

void E_I_Summation(std::int16_t*** Input, int t, double** kernel_E, double** kernel_I, int width_E, int width_I, \
    double** Summation_Layer, double Weight_E, double Weight_I)
{
    double** E_Layer;
    double** I_Layer;
    double E_Cov_Pixel;
    double I_Cov_Pixel;
    double S_Layer_Pixel;

    int radius_E = width_E / 2; // 3/2 = 1
    int radius_I = width_I / 2; // 5/2 = 2

    int i, j, m, n;
    int x, y;

    Allocate2DLayer(&E_Layer, Height, Width);
    Allocate2DLayer(&I_Layer, Height, Width);

    for (i = 0; i < Height; i++)
    {
        for (j = 0; j < Width; j++)
        {
            // E Layer Convolution : 
            for (m = 0; m < width_E; m++)
            {
                //printf("\nrow %d : \n", m);
                y = i - radius_E + m;
                for (n = 0; n < width_E; n++)
                {

                    x = j - radius_E + n;

                    /*
                    // if exceeding range, let it equal to margin pixel value instead of 0.
                    while (y < 0)
                    {
                    y++;
                    }
                    while (y >= Height)
                    {
                    y--;
                    }
                    while (x < 0)
                    {
                    x++;
                    }
                    while (x >= Width)
                    {
                    x--;
                    }
                    */

                    if (y < 0 || y >= Height || x < 0 || x >= Width) {
                        // mediate ...
                        continue;
                    }

                    /*
                    printf("\n\n   y:%d x:%d t:%d   m:%d n:%d\n", y, x, t, m, n);
                    printf("[img: %d]  x  ", Input[t][y][x]);
                    printf("[kernel: %f]", kernel_E[m][n]);
                    */

                    E_Cov_Pixel = (double)Input[t][y][x] * kernel_E[m][n]; //core
                    // sum 3 x 3 :
                    E_Layer[i][j] += E_Cov_Pixel;

                    // abandon edge of the view ...
                    if (y < width_I || y >= Height - width_I || x < width_I || x >= Width - width_I) {
                        E_Layer[i][j] = 0;
                    }


                }
            }
            //printf("\nE_Cov pixel[%d][%d] over ...\n\n",i, j);


            // I Layer Convolution : 
            for (m = 0; m < width_I; m++)
            {
                //printf("\nrow %d : \n", m);
                y = i - radius_I + m;
                for (n = 0; n < width_I; n++)
                {
                    x = j - radius_I + n;

                    /*
                    // if exceeding range, let it equal to margin pixel value instead of 0.
                    while (y < 0)
                    {
                    y++;
                    }
                    while (y >= Height)
                    {
                    y--;
                    }
                    while (x < 0)
                    {
                    x++;
                    }
                    while (x >= Width)
                    {
                    x--;
                    }
                    */

                    if (y < 0 || y >= Height || x < 0 || x >= Width) {
                        // mediate ...
                        continue;
                    }

                    /*
                    printf("\n\n   y:%d x:%d t:%d   m:%d n:%d\n", y, x, t, m, n);
                    printf("[img: %d]  x  ", Input[t][y][x]);//
                    printf("[kernel: %f]", kernel_I[m][n]);
                    */

                    I_Cov_Pixel = (double)Input[!t][y][x] * kernel_I[m][n]; // t-1 time inhibition information
                    // sum 5 x 5 :
                    I_Layer[i][j] += I_Cov_Pixel;

                    // abandon edge of the view ...
                    if (y < width_I || y >= Height-width_I || x < width_I || x >= Width- width_I) {
                        I_Layer[i][j] = 0;
                    }

                }
            }
            //printf("\nI_Cov pixel[%d][%d] over ...\n\n", i, j);


            // Summation :
            S_Layer_Pixel = Weight_E * E_Layer[i][j] - Weight_I * I_Layer[i][j];
            if (E_Layer[i][j] >= 0) {
                if (S_Layer_Pixel >= 0)
                    Summation_Layer[i][j] = S_Layer_Pixel;
                else // E > 0, S < 0
                    Summation_Layer[i][j] = 0;
            }
            else {
                if (S_Layer_Pixel <= 0)
                    Summation_Layer[i][j] = S_Layer_Pixel;
                else // E < 0, S > 0
                    Summation_Layer[i][j] = 0;
            }



        }
    }



    Free2DLayer(&E_Layer, Height);
    Free2DLayer(&I_Layer, Height);
}

double NormalizeContrast(double*** ON_OFF_Channel, int i, int j, int t, double** kernel, int width_cn, double coe_cn, double cn_threshold)
{
    double sum = 0;
    int radius = width_cn / 2;
    int y = 0;
    int x = 0;

    // Sum 11 x 11 :
    for (int m = -radius; m < radius + 1; m++)
    {
        y = i + m;
        for (int n = -radius; n < radius + 1; n++)
        {
            x = j + n;

            if (y < 0 || y >= Height || x < 0 || x >= Width)
                continue;

            sum += ON_OFF_Channel[t][i][j] * kernel[m + radius][n + radius];
        }
    }

    // Activation :
    // return tanh(ON_OFF_Channel[t][i][j] / (coe_cn + sum) );
    double compressed = tanh(ON_OFF_Channel[t][i][j] / (coe_cn + sum));
    if (compressed > cn_threshold) {
        // mediate ...
        //printf("\n pixel:  %f   sum+coe:  %f   tanh(pixel/(sum+coe):  %f ", ON_OFF_Channel[t][i][j], sum+coe_cn, compressed);
    }
    //double compressed = tanh(sum/coe_cn);
    if (compressed > cn_threshold)
        return compressed;
    else
        return 0;
}


// LPLC2 : 
bool LPLC2_Init(LPLC2_pControlTypedef* hLPLC2)// Inpt ptr so as to change values in 'hLPLC'
{
    // simplize ptr passing : 
    LPLC2struct_Params* Params = &hLPLC2->aModel.Params;
    LPLC2struct_Layers* Layers = &hLPLC2->aModel.Layers;
    LPLC2struct_Results* Results = &hLPLC2->aModel.Results;
    printf("\n\nLPLC2 Params Initialization Begin:\r\n\n");

    // LGMD general handle params :
    hLPLC2->processCount = 1;
    hLPLC2->processCountLast = 0;
    hLPLC2->Enable = 1;
    hLPLC2->status = 0;
    hLPLC2->processRate = 30; // 30 fps
    hLPLC2->currentImage = 0;
    hLPLC2->currentDiffImage = 0;
    hLPLC2->previousDiffImage = 0;


    // LPLC2 params initialization : 
    Params->clip = 0;
    Params->interval = 1000 / FPS;// ms
    Params->tau_hp = 500; // ms
    Params->delay_hp = Params->tau_hp / (Params->tau_hp + Params->interval);  // tau_hp ^, delay_hp ^, cur ^


    Params->std_exc = 10;
    Params->std_inh = 20;
    Params->std_cn = 20;

    Params->width_exc = 7;
    Params->width_inh = 11;
    Params->width_cn = 5;

    Params->coe_Weight_E = 1;
    Params->coe_Weight_I = 1.2;
    // ramain in ON/OFF half-wave rectifying, causing Afterimage
    Params->remain_rec = 0.1;
    // compressed = tanh( sum / coe_cn );
    Params->coe_cn = 0.2; 

    // threshold in compressed normalization, greatly reducing noice after normallization
    
    // synthetic video : 
    //Params->cn_threshold = 0.75; // outdoor_1 : 0.67, outdoor_2 : 0.7, outdoor_3 : 
    
    // UAV_outdoor : 
    //Params->cn_threshold = 0.746 ; // bird_attack
    //Params->cn_threshold = 0.7 ; // bird_attack_2
    //Params->cn_threshold = 0.72 ; // helmet_attack
    Params->cn_threshold = 0.75 ; // car_attack




    // EMD delay information : 
    Params->tau_lp = 30; // FATAL
    Params->delay_lp = Params->interval / (Params->tau_lp + Params->interval);// tau_lp ^, delay_lp !, cur !
    // Sampling Distance
    Params->sd = 13; //FATAL
    // if rightward motion, emd_bias surpress UP/DOWN signal
    Params->emd_bias = 1.5;


    // Dynamic Delay Mechanism : 
    Params->sd_connected = 5;
    Params->sd_step = 1; // FATAL
    Params->tau_lp_dynamic[0] = 80; // ms
    printf("Dynamic Delay:\nsd_connected: %d   sd_step: %d\n", Params->sd_connected, Params->sd_step );
    for (int i = 0; i < Params->sd_connected; i++)
    {
        // delay: 40, 30, 20, 10, 10, 10, 10 ...
        Params->tau_lp_dynamic[i] = Params->tau_lp_dynamic[0] - i * 10;
        if (Params->tau_lp_dynamic[i] <= 10)
            Params->tau_lp_dynamic[i] = 10;

        Params->delay_lp_dynamic[i] = Params->interval / (Params->tau_lp_dynamic[i] + Params->interval);
        printf("\ntau_lp_dynamic[%d]: %d\tdelay_lp_dynamic[%d]: %f\n", i, Params->tau_lp_dynamic[i], i, Params->delay_lp_dynamic[i]);
    }

    // to avoid 'sd' bigger than 'sd_connected*sd_step', breaking 'for loop' based on 'sd_connected*sd_step' :
    if (!USE_DYNAMIC_DELAY) {
        Params->sd_connected = Params->sd;
        Params->sd_step = 1;
    }


    Params->T4_lp = 30;
    Params->T5_lp = 30;
    Params->delay_T4 = Params->interval / (Params->T4_lp + Params->interval);
    Params->delay_T5 = Params->interval / (Params->T5_lp + Params->interval);
    printf("\n\n\nOther Delays(ms): hp: %f, lp:%f, T4: %f, T5: %f \n", Params->delay_hp, Params->delay_lp, Params->delay_T4, Params->delay_T5);


    Params->exp_ON = 0.9;
    Params->exp_OFF = 0.5;


    Results->Attention_Coordinate[0] = Height / 2;
    Results->Attention_Coordinate[1] = Width / 2;

    Results->LPLC2_Quadrant_Output[0] = 0;
    Results->LPLC2_Quadrant_Output[1] = 0;
    Results->LPLC2_Quadrant_Output[2] = 0;
    Results->LPLC2_Quadrant_Output[3] = 0;
    memset(Results->LPLC2, 0, sizeof(Results->LPLC2) );


    // Multi Attention Mechanism
    Params->Judge_Field_size = 150 ;
    Params->RF_size = 150 ;

    Results->MaxMag = 0 ;
    Results->MaxMag_x = 0 ;
    Results->MaxMag_y = 0 ;

    Params->Magnify = 10 ;

    // If d frames accumulated output < T_af : abandon this AF
    Params->T_af = 10000 ; 
    Params->d = 5 ;

    Params->T_collision = 1000 ;

    Params->T_lplc2 = 50 ; // Threshold for 4 quadrants 


    printf("\n\nLPLC2 Params Initialization Over......\r\n\n\n\n");






    // Allocate memory at HEAP for layers: 
    printf("\n\n\nAllocate Memory at HEAP for Layers Begin :\r\n");
    // hLPLC2->aModel.Layers.{Layer_Name}[deep][Height][Width] 
    Allocate3DLayer(&Layers->Diff_Img, 2, Height, Width); // uint8_t

    Allocate2DLayer(&Layers->Kernel_GB_Exc, Params->width_exc, Params->width_exc);
    Allocate2DLayer(&Layers->Kernel_GB_Inh, Params->width_inh, Params->width_inh);
    Allocate2DLayer(&Layers->Summation, Height, Width);

    Allocate3DLayer(&Layers->ON, 2, Height, Width);
    Allocate3DLayer(&Layers->OFF, 2, Height, Width);

    Allocate2DLayer(&Layers->Kernel_Contrast, Params->width_cn, Params->width_cn);
    Allocate3DLayer(&Layers->ON_Compressed, 2, Height, Width);
    Allocate3DLayer(&Layers->OFF_Compressed, 2, Height, Width);
    Allocate3DLayer(&Layers->ON_Delay, 2, Height, Width);
    Allocate3DLayer(&Layers->OFF_Delay, 2, Height, Width);

    Allocate4DLayer(&Layers->T4s, 2, Height, Width, 4);
    Allocate4DLayer(&Layers->T5s, 2, Height, Width, 4);
    Allocate3DLayer(&Layers->LM, Height, Width, 4);

    Allocate2DLayer(&Layers->LPLC2s_mag, Height, Width);
    Allocate2DLayer(&Layers->LPLC2s_direction, Height, Width);




    MakeGaussianKernel(Params->std_exc, Params->width_exc, &Layers->Kernel_GB_Exc);
    MakeGaussianKernel(Params->std_inh, Params->width_inh, &Layers->Kernel_GB_Inh);
    MakeGaussianKernel(Params->std_cn, Params->width_cn, &Layers->Kernel_Contrast);


    Results->ExistingAFs = createFieldSet() ;
    Results->ExistedAFs = createFieldSet() ;
    Results->FatalAFs = createFieldSet() ;

    printf("\n\n\nAllocate Memory at HEAP for Layers Over......\r\n\n\n\n\n\n\n");
    return 1;
}

bool LPLC2_Calculating(LPLC2_pControlTypedef* hLPLC2, std::vector<std::vector<std::vector<uint8_t>>> Vedio)
{
    // simplize ptr passing : 
    LPLC2struct_Params* Params = &hLPLC2->aModel.Params;
    LPLC2struct_Layers* Layers = &hLPLC2->aModel.Layers;
    LPLC2struct_Results* Results = &hLPLC2->aModel.Results;


    // P Layer : 
    for (int i = 0; i < Height; i++)
    {
        //printf("\nraw%d\n", i);
        for (int j = 0; j < Width; ++j)
        {

            std::uint8_t* cur_pixel;
            std::uint8_t* pre_pixel;
            std::int16_t* Diff_pixel;

            Diff_pixel = &hLPLC2->aModel.Layers.Diff_Img[hLPLC2->currentDiffImage][i][j];
            cur_pixel = & Vedio[i][j][*hLPLC2->hFrameCount];
            pre_pixel = & Vedio[i][j][*hLPLC2->hFrameCount - 1];

            /* // using uint8_t : 
            double Diff = *cur_pixel - *pre_pixel;
            if (Diff > 127) {
            Diff = 127;
            }
            else if (Diff < -127) {
            Diff = -127;
            }
            *Diff_pixel = Diff; // -127 ~ 127
            */                                        // use int16_t to storage Diff_Img instead of int8_t, \
                        storaging more details of difference between two consecutive frames, making gaussian_blur easier.

            // High-pass filter : 
            std::int16_t Diff = (std::int16_t)(*cur_pixel - *pre_pixel);

            //*Diff_pixel = Diff + (std::int16_t)(*cur_pixel);
            *Diff_pixel = Diff;

            //printf("%d", hLPLC2->aModel.Layers.Diff_Img[hLPLC2->currentDiffImage][i][j]);
        }
    }// now we've got Diff_Img[Height][Width][1]


     // E, I, S Layer : 'vDoG' algorithm
    E_I_Summation(Layers->Diff_Img, 1, Layers->Kernel_GB_Exc, Layers->Kernel_GB_Inh, 3, 5, Layers->Summation, Params->coe_Weight_E, Params->coe_Weight_I );


    // ON/OFF Channel : 
    for (int y = 0; y < Height; y++)
    {
        for (int x = 0; x < Width; x++)
        {
            //printf("%f", Layers->OFF[hLPLC2->previousDiffImage][y][x]);
            //printf("%f", Layers->Summation[y][x]);
            Layers->ON[hLPLC2->currentDiffImage][y][x] = \
                Halfwave_ON(Layers->Summation[y][x], Layers->ON[hLPLC2->previousDiffImage][y][x], Params->clip, Params->remain_rec);
            Layers->OFF[hLPLC2->currentDiffImage][y][x] = \
                Halfwave_OFF(Layers->Summation[y][x], Layers->OFF[hLPLC2->previousDiffImage][y][x], Params->clip, Params->remain_rec);
        }
    }


    // Compress ON/OFF, Delay Compressed Signal : 
    for (int y = 0; y < Height; y++)
    {
        for (int x = 0; x < Width; x++)
        {
            // Tanh Compression : both ON and OFF pathways
            Layers->ON_Compressed[hLPLC2->currentDiffImage][y][x] = \
                NormalizeContrast(Layers->ON, y, x, hLPLC2->currentDiffImage, Layers->Kernel_Contrast, Params->width_cn, Params->coe_cn, Params->cn_threshold);
            Layers->OFF_Compressed[hLPLC2->currentDiffImage][y][x] = \
                NormalizeContrast(Layers->OFF, y, x, hLPLC2->currentDiffImage, Layers->Kernel_Contrast, Params->width_cn, Params->coe_cn, Params->cn_threshold);


            // Statistic Low-pass Delay :
            if (!USE_DYNAMIC_DELAY)
            {
                Layers->ON_Delay[hLPLC2->currentDiffImage][y][x] = \
                    Lowpass(Layers->ON_Compressed[hLPLC2->currentDiffImage][y][x], Layers->ON_Compressed[hLPLC2->previousDiffImage][y][x], Params->delay_lp);
                Layers->OFF_Delay[hLPLC2->currentDiffImage][y][x] = \
                    Lowpass(Layers->OFF_Compressed[hLPLC2->currentDiffImage][y][x], Layers->OFF_Compressed[hLPLC2->previousDiffImage][y][x], Params->delay_lp);
            }

        }
    }


    // Motion Correlation Pathway & LPLC2 projection neuron
    bool cur = hLPLC2->currentDiffImage;
    bool pre = !cur;
    int sd = Params->sd;
    double bias = Params->emd_bias;
    double right, left, down, up, horizonal, vertical;
    Results->MaxMag = 0;
    Results->MaxMag_y = 0 ;
    Results->MaxMag_x = 0 ;
    Results->BEYOND = 0 ;
    Results->BEYOND_y = 0 ;
    Results->BEYOND_x = 0 ;
    FieldSet* ExistingAFs = Results->ExistingAFs ;
    FieldSet* ExistedAFs = Results->ExistedAFs ;
    FieldSet* FatalAFs = Results->FatalAFs ;
    double coeM = Params->Magnify ;

    if(USE_MULTI_ATTENTION_MECHANISM == 0){

        Results->LPLC2_Quadrant_Output[0] = 0;
        Results->LPLC2_Quadrant_Output[1] = 0;
        Results->LPLC2_Quadrant_Output[2] = 0;
        Results->LPLC2_Quadrant_Output[3] = 0;
        Results->LPLC2_Output[*hLPLC2->hFrameCount] = 0;
        // for single-attention mechanism to find the centroid : 
        // centroid_x = numerator_x / denominator_x 
        double numerator_x = 0, numerator_y = 0, denominator_x = 0, denominator_y = 0; 

        if (USE_ATTENTION_MECHANISM){
            for (int y = (Params->sd_connected) * (Params->sd_step); y < Height - (Params->sd_connected) * (Params->sd_step); y++)
            {
                for (int x = (Params->sd_connected) * (Params->sd_step); x < Width - (Params->sd_connected) * (Params->sd_step); x++)
                {
                    // Rightward : 0   
                    Layers->T4s[cur][y][x][0] = 0 ; // memset, clear pre_frame remaining signal
                    Layers->T5s[cur][y][x][0] = 0 ;
                    // Leftward : 1             
                    Layers->T4s[cur][y][x][1] = 0 ;
                    Layers->T5s[cur][y][x][1] = 0 ;
                    // Downward : 2             
                    Layers->T4s[cur][y][x][2] = 0 ;
                    Layers->T5s[cur][y][x][2] = 0 ;
                    // Upward : 3               
                    Layers->T4s[cur][y][x][3] = 0 ;
                    Layers->T5s[cur][y][x][3] = 0 ;
                    int count = 1;

                    if (USE_DYNAMIC_DELAY) {
                        // Dynamic Delay Mechanism :
                        for (int distance = Params->sd_step; distance <= (Params->sd_connected) * (Params->sd_step); distance += Params->sd_step)
                        {
                            //if( y == (Params->sd_connected)*(Params->sd_step) && x == (Params->sd_connected)*(Params->sd_step) )
                            //    printf("\nNeuron connected: %d\n", count++ );

                            // Dynamic Delay :
                            Layers->ON_Delay[cur][y][x] = Lowpass(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[pre][y][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                            Layers->ON_Delay[cur][y][x + distance] = Lowpass(Layers->ON_Compressed[cur][y][x + distance], Layers->ON_Compressed[pre][y][x + distance], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                            Layers->ON_Delay[cur][y][x - distance] = Lowpass(Layers->ON_Compressed[cur][y][x - distance], Layers->ON_Compressed[pre][y][x - distance], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                            Layers->ON_Delay[cur][y + distance][x] = Lowpass(Layers->ON_Compressed[cur][y + distance][x], Layers->ON_Compressed[pre][y + distance][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                            Layers->ON_Delay[cur][y - distance][x] = Lowpass(Layers->ON_Compressed[cur][y - distance][x], Layers->ON_Compressed[pre][y - distance][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);

                            Layers->OFF_Delay[cur][y][x] = Lowpass(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[pre][y][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                            Layers->OFF_Delay[cur][y][x + distance] = Lowpass(Layers->OFF_Compressed[cur][y][x + distance], Layers->OFF_Compressed[pre][y][x + distance], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                            Layers->OFF_Delay[cur][y][x - distance] = Lowpass(Layers->OFF_Compressed[cur][y][x - distance], Layers->OFF_Compressed[pre][y][x - distance], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                            Layers->OFF_Delay[cur][y + distance][x] = Lowpass(Layers->OFF_Compressed[cur][y + distance][x], Layers->OFF_Compressed[pre][y + distance][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                            Layers->OFF_Delay[cur][y - distance][x] = Lowpass(Layers->OFF_Compressed[cur][y - distance][x], Layers->OFF_Compressed[pre][y - distance][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);


                            // Cov_TripleCorrelation_Bias, bias = 1, using compressed channel.
                            // Rightward : 0   
                            Layers->T4s[cur][y][x][0] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x + distance], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x + distance], bias);
                            Layers->T5s[cur][y][x][0] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x + distance], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x + distance], bias);
                            // Leftward : 1           +  Cov_Triple                                                                                      distance                                                             distance
                            Layers->T4s[cur][y][x][1] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x - distance], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x - distance], bias);
                            Layers->T5s[cur][y][x][1] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x - distance], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x - distance], bias);
                            // Downward : 2           +  Cov_Triple                                                                     
                            Layers->T4s[cur][y][x][2] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y + distance][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y + distance][x], bias);
                            Layers->T5s[cur][y][x][2] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y + distance][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y + distance][x], bias);
                            // Upward : 3             +  Cov_Triple                                                                                   distance                                                             distance
                            Layers->T4s[cur][y][x][3] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y - distance][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y - distance][x], bias);
                            Layers->T5s[cur][y][x][3] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y - distance][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y - distance][x], bias);

                            // using pre_ON/OFF as delay signal instead of low-pass filter : 
                            /*
                            // Rightward : 0
                            Layers->T4s[cur][y][x][0] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x + distance],    Layers->ON_Compressed[!cur][y][x],  Layers->ON_Compressed[!cur][y][x + distance], bias);
                            Layers->T5s[cur][y][x][0] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x + distance], Layers->OFF_Compressed[!cur][y][x], Layers->OFF_Compressed[!cur][y][x + distance], bias);
                            // Leftward : 1           +  Cov_Triple                                                                                      distance               _Compressed !                       _Compressed !
                            Layers->T4s[cur][y][x][1] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x - distance], Layers->ON_Compressed[!cur][y][x], Layers->ON_Compressed[!cur][y][x - distance], bias);
                            Layers->T5s[cur][y][x][1] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x - distance], Layers->OFF_Compressed[!cur][y][x], Layers->OFF_Compressed[!cur][y][x - distance], bias);
                            // Downward : 2           +  Cov_Triple                                                                                                             _Compressed !                       _Compressed !
                            Layers->T4s[cur][y][x][2] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y + distance][x], Layers->ON_Compressed[!cur][y][x], Layers->ON_Compressed[!cur][y + distance][x], bias);
                            Layers->T5s[cur][y][x][2] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y + distance][x], Layers->OFF_Compressed[!cur][y][x], Layers->OFF_Compressed[!cur][y + distance][x], bias);
                            // Upward : 3             +  Cov_Triple                                                                                   distance                  _Compressed !                       _Compressed !
                            Layers->T4s[cur][y][x][3] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y - distance][x], Layers->ON_Compressed[!cur][y][x], Layers->ON_Compressed[!cur][y - distance][x], bias);
                            Layers->T5s[cur][y][x][3] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y - distance][x], Layers->OFF_Compressed[!cur][y][x], Layers->OFF_Compressed[!cur][y - distance][x], bias);
                            */
                        }

                    }else{// #define USE_DYNAMIC_DELAY 0, connected neuron number = 2

                        // EMD with Raw ON/OFF Signal : 
                        /*
                        // Rightward : 0
                        Layers->T4s[cur][y][x][0] = HRCorrelation_Bias (Layers->ON[cur][y][x], Layers->ON[cur][y][x + sd], Layers->ON[pre][y][x], Layers->ON[pre][y][x + sd], bias);
                        Layers->T5s[cur][y][x][0] = HRCorrelation_Bias(Layers->OFF[cur][y][x],Layers->OFF[cur][y][x + sd],Layers->OFF[pre][y][x],Layers->OFF[pre][y][x + sd], bias);
                        // Leftward : 1                                                                                               pre                    pre
                        Layers->T4s[cur][y][x][1] = HRCorrelation_Bias (Layers->ON[cur][y][x], Layers->ON[cur][y][x - sd], Layers->ON[pre][y][x], Layers->ON[pre][y][x - sd], bias);
                        Layers->T5s[cur][y][x][1] = HRCorrelation_Bias(Layers->OFF[cur][y][x],Layers->OFF[cur][y][x - sd],Layers->OFF[pre][y][x],Layers->OFF[pre][y][x - sd], bias);
                        // Downward : 2                                                                                               pre                    pre
                        Layers->T4s[cur][y][x][2] = HRCorrelation_Bias (Layers->ON[cur][y][x], Layers->ON[cur][y + sd][x], Layers->ON[pre][y][x], Layers->ON[pre][y + sd][x], bias);
                        Layers->T5s[cur][y][x][2] = HRCorrelation_Bias(Layers->OFF[cur][y][x],Layers->OFF[cur][y + sd][x],Layers->OFF[pre][y][x],Layers->OFF[pre][y + sd][x], bias);
                        // Upward : 3                                                                                                 pre                    pre
                        Layers->T4s[cur][y][x][3] = HRCorrelation_Bias (Layers->ON[cur][y][x], Layers->ON[cur][y - sd][x], Layers->ON[pre][y][x], Layers->ON[pre][y - sd][x], bias);
                        Layers->T5s[cur][y][x][3] = HRCorrelation_Bias(Layers->OFF[cur][y][x],Layers->OFF[cur][y - sd][x],Layers->OFF[pre][y][x],Layers->OFF[pre][y - sd][x], bias);
                        */

                        // EMD with Compressed Signal : 
                        /*
                        // HRCorrelation_Bias, bias = 1
                        // Rightward : 0
                        Layers->T4s[cur][y][x][0] = HRCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x + sd], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x + sd], bias);
                        Layers->T5s[cur][y][x][0] = HRCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x + sd], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x + sd], bias);
                        // Leftward : 1
                        Layers->T4s[cur][y][x][1] = HRCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x - sd], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x - sd], bias);
                        Layers->T5s[cur][y][x][1] = HRCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x - sd], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x - sd], bias);
                        // Downward : 2
                        Layers->T4s[cur][y][x][2] = HRCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y + sd][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y + sd][x], bias);
                        Layers->T5s[cur][y][x][2] = HRCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y + sd][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y + sd][x], bias);
                        // Upward : 3
                        Layers->T4s[cur][y][x][3] = HRCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y - sd][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y - sd][x], bias);
                        Layers->T5s[cur][y][x][3] = HRCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y - sd][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y - sd][x], bias);
                        */

                        // Div_TripleCorrelation_Bias�� bias = 1
                        /*
                        // Rightward : 0
                        Layers->T4s[cur][y][x][0] = Div_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x + sd], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x + sd], bias);
                        Layers->T5s[cur][y][x][0] = Div_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x + sd], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x + sd], bias);
                        // Leftward : 1             Div_Triple
                        Layers->T4s[cur][y][x][1] = Div_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x - sd], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x - sd], bias);
                        Layers->T5s[cur][y][x][1] = Div_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x - sd], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x - sd], bias);
                        // Downward : 2             Div_Triple
                        Layers->T4s[cur][y][x][2] = Div_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y + sd][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y + sd][x], bias);
                        Layers->T5s[cur][y][x][2] = Div_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y + sd][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y + sd][x], bias);
                        // Upward : 3               Div_Triple
                        Layers->T4s[cur][y][x][3] = Div_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y - sd][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y - sd][x], bias);
                        Layers->T5s[cur][y][x][3] = Div_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y - sd][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y - sd][x], bias);
                        */

                        // Cov_TripleCorrelation_Bias, bias = 1

                        // Rightward : 0
                        Layers->T4s[cur][y][x][0] = Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x + sd], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x + sd], bias);
                        Layers->T5s[cur][y][x][0] = Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x + sd], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x + sd], bias);
                        // Lef tward : 1             Cov_Triple
                        Layers->T4s[cur][y][x][1] = Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x - sd], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x - sd], bias);
                        Layers->T5s[cur][y][x][1] = Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x - sd], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x - sd], bias);
                        // Downward : 2             Cov_Triple
                        Layers->T4s[cur][y][x][2] = Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y + sd][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y + sd][x], bias);
                        Layers->T5s[cur][y][x][2] = Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y + sd][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y + sd][x], bias);
                        // Upward : 3               Cov_Triple
                        Layers->T4s[cur][y][x][3] = Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y - sd][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y - sd][x], bias);
                        Layers->T5s[cur][y][x][3] = Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y - sd][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y - sd][x], bias);
                    }

                    // Lowpass of T4/T5 neurons :
                    Layers->T4s[cur][y][x][0] = Lowpass(Layers->T4s[cur][y][x][0], Layers->T4s[pre][y][x][0], Params->delay_T4);
                    Layers->T5s[cur][y][x][0] = Lowpass(Layers->T5s[cur][y][x][0], Layers->T5s[pre][y][x][0], Params->delay_T5);

                    Layers->T4s[cur][y][x][1] = Lowpass(Layers->T4s[cur][y][x][1], Layers->T4s[pre][y][x][1], Params->delay_T4);
                    Layers->T5s[cur][y][x][1] = Lowpass(Layers->T5s[cur][y][x][1], Layers->T5s[pre][y][x][1], Params->delay_T5);

                    Layers->T4s[cur][y][x][2] = Lowpass(Layers->T4s[cur][y][x][2], Layers->T4s[pre][y][x][2], Params->delay_T4);
                    Layers->T5s[cur][y][x][2] = Lowpass(Layers->T5s[cur][y][x][2], Layers->T5s[pre][y][x][2], Params->delay_T5);

                    Layers->T4s[cur][y][x][3] = Lowpass(Layers->T4s[cur][y][x][3], Layers->T4s[pre][y][x][3], Params->delay_T4);
                    Layers->T5s[cur][y][x][3] = Lowpass(Layers->T5s[cur][y][x][3], Layers->T5s[pre][y][x][3], Params->delay_T5);


                    // ON/OFF Intigration 
                    right = ON_OFF_ChannelSummation(Layers->T4s[cur][y][x][0], Layers->T5s[cur][y][x][0], Params->exp_ON, Params->exp_OFF);
                    left  = ON_OFF_ChannelSummation(Layers->T4s[cur][y][x][1], Layers->T5s[cur][y][x][1], Params->exp_ON, Params->exp_OFF);
                    down  = ON_OFF_ChannelSummation(Layers->T4s[cur][y][x][2], Layers->T5s[cur][y][x][2], Params->exp_ON, Params->exp_OFF);
                    up    = ON_OFF_ChannelSummation(Layers->T4s[cur][y][x][3], Layers->T5s[cur][y][x][3], Params->exp_ON, Params->exp_OFF);


                    // Local Motion Integration :
                    Layers->LM[y][x][0] = Leaky_ReLU(right - left);
                    Layers->LM[y][x][1] = Leaky_ReLU(left - right);
                    Layers->LM[y][x][2] = Leaky_ReLU(down - up);
                    Layers->LM[y][x][3] = Leaky_ReLU(up - down);


                    // Local Direction : 
                    horizonal = (right > left) ? right : -left;
                    vertical = (up > down) ? up : -down;
                    Layers->LPLC2s_direction[y][x] = atan2(vertical, horizonal) * 180 / M_PI;


                    // Local Magnitude : 
                    Layers->LPLC2s_mag[y][x] = sqrt(pow(right + left, 2) + pow(up + down, 2) );
                    if (Results->MaxMag < Layers->LPLC2s_mag[y][x])
                    {
                        Results->MaxMag = Layers->LPLC2s_mag[y][x];
                        Results->MaxMag_y = y ;
                        Results->MaxMag_x = x ;
                    }


                    // calculate integration of centroid :
                    numerator_y += Layers->LPLC2s_mag[y][x] * y;
                    denominator_y += Layers->LPLC2s_mag[y][x];
                    numerator_x += Layers->LPLC2s_mag[y][x] * x;
                    denominator_x += Layers->LPLC2s_mag[y][x];

                }
            }// Traverse pixels over

             // Find Attention Centroid :
            Results->Attention_Coordinate[0] = numerator_y / denominator_y;
            Results->Attention_Coordinate[1] = numerator_x / denominator_x;
            printf("\nCurrent Attention Centroid (y , x): (%d , %d)\n", Results->Attention_Coordinate[0], Results->Attention_Coordinate[1]);


            // Integrate motion information from 4-quadrants of the whole visual field: 
            for (int y = (Params->sd_connected) * (Params->sd_step); y < Height - (Params->sd_connected) * (Params->sd_step); y++)
            {
                for (int x = (Params->sd_connected) * (Params->sd_step); x < Width - (Params->sd_connected) * (Params->sd_step); x++)
                {

                    // Regional integration of visual projection neurons : 
                    if (y <= Results->Attention_Coordinate[0] && x > Results->Attention_Coordinate[1]) {
                        Results->LPLC2_Quadrant_Output[0] += Layers->LM[y][x][0];
                        Results->LPLC2_Quadrant_Output[0] += Layers->LM[y][x][3];

                    }
                    else if (y < Results->Attention_Coordinate[0] && x <= Results->Attention_Coordinate[1]) {
                        Results->LPLC2_Quadrant_Output[1] += Layers->LM[y][x][1];
                        Results->LPLC2_Quadrant_Output[1] += Layers->LM[y][x][3];

                    }
                    else if (y >= Results->Attention_Coordinate[0] && x < Results->Attention_Coordinate[1]) {
                        Results->LPLC2_Quadrant_Output[2] += Layers->LM[y][x][1];
                        Results->LPLC2_Quadrant_Output[2] += Layers->LM[y][x][2];

                    }
                    else {// y > AC[0], x >= AC[1]  
                        Results->LPLC2_Quadrant_Output[3] += Layers->LM[y][x][0];
                        Results->LPLC2_Quadrant_Output[3] += Layers->LM[y][x][2];
                    }
                }

            }

            // Integrate motion information in the LPLC2 Attention zone : 
            /*
            for(int y = Results->Attention_Coordinate[0]-(Params->RF_size/2); y < Results->Attention_Coordinate[0]+(Params->RF_size/2); y ++ )
            {
            for(int x = Results->Attention_Coordinate[1]-(Params->RF_size/2); x < Results->Attention_Coordinate[1]+(Params->RF_size/2); x ++ )
            {
            if (y < 0 || y >= Height || x < 0 || x >= Width){
            continue;
            } // avoid out of range


            // Regional integration of visual projection neurons : 
            if (y <= Results->Attention_Coordinate[0] && x > Results->Attention_Coordinate[1]) {
            Results->LPLC2_Quadrant_Output[0] += Layers->LM[y][x][0];
            Results->LPLC2_Quadrant_Output[0] += Layers->LM[y][x][3];

            }
            else if (y < Results->Attention_Coordinate[0] && x <= Results->Attention_Coordinate[1]) {
            Results->LPLC2_Quadrant_Output[1] += Layers->LM[y][x][1];
            Results->LPLC2_Quadrant_Output[1] += Layers->LM[y][x][3];

            }
            else if (y >= Results->Attention_Coordinate[0] && x < Results->Attention_Coordinate[1]) {
            Results->LPLC2_Quadrant_Output[2] += Layers->LM[y][x][1];
            Results->LPLC2_Quadrant_Output[2] += Layers->LM[y][x][2];

            }
            else {// y > AC[0], x >= AC[1]  
            Results->LPLC2_Quadrant_Output[3] += Layers->LM[y][x][0];
            Results->LPLC2_Quadrant_Output[3] += Layers->LM[y][x][2];
            }
            }

            }
            */
            printf("\nQuadrant output:\n1: %f  2: %f  3: %f  4 : %f\n", Results->LPLC2_Quadrant_Output[0], Results->LPLC2_Quadrant_Output[1], Results->LPLC2_Quadrant_Output[2], Results->LPLC2_Quadrant_Output[3]);



        }else{// USE_ATTENTION_MECHANISM 0 , image center is the view center...
            for (int y = (Params->sd_connected) * (Params->sd_step); y < Height - (Params->sd_connected) * (Params->sd_step); y++)
            {
                for (int x = (Params->sd_connected) * (Params->sd_step); x < Width - (Params->sd_connected) * (Params->sd_step); x++)
                {
                    // Rightward : 0   
                    Layers->T4s[cur][y][x][0] = 0; // memset, clear pre_frame remaining signal
                    Layers->T5s[cur][y][x][0] = 0;
                    // Leftward : 1             
                    Layers->T4s[cur][y][x][1] = 0;
                    Layers->T5s[cur][y][x][1] = 0;
                    // Downward : 2             
                    Layers->T4s[cur][y][x][2] = 0;
                    Layers->T5s[cur][y][x][2] = 0;
                    // Upward : 3               
                    Layers->T4s[cur][y][x][3] = 0;
                    Layers->T5s[cur][y][x][3] = 0;
                    int count = 1;

                    if (USE_DYNAMIC_DELAY) {
                        // Dynamic Delay Mechanism :
                        for (int distance = Params->sd_step; distance <= (Params->sd_connected) * (Params->sd_step); distance += Params->sd_step)
                        {
                            //if( y == (Params->sd_connected)*(Params->sd_step) && x == (Params->sd_connected)*(Params->sd_step) )
                            //    printf("\nNeuron connected: %d\n", count++ );

                            // Dynamic Delay :
                            Layers->ON_Delay[cur][y][x] = Lowpass(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[pre][y][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                            Layers->ON_Delay[cur][y][x + distance] = Lowpass(Layers->ON_Compressed[cur][y][x + distance], Layers->ON_Compressed[pre][y][x + distance], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                            Layers->ON_Delay[cur][y][x - distance] = Lowpass(Layers->ON_Compressed[cur][y][x - distance], Layers->ON_Compressed[pre][y][x - distance], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                            Layers->ON_Delay[cur][y + distance][x] = Lowpass(Layers->ON_Compressed[cur][y + distance][x], Layers->ON_Compressed[pre][y + distance][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                            Layers->ON_Delay[cur][y - distance][x] = Lowpass(Layers->ON_Compressed[cur][y - distance][x], Layers->ON_Compressed[pre][y - distance][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);

                            Layers->OFF_Delay[cur][y][x] = Lowpass(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[pre][y][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                            Layers->OFF_Delay[cur][y][x + distance] = Lowpass(Layers->OFF_Compressed[cur][y][x + distance], Layers->OFF_Compressed[pre][y][x + distance], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                            Layers->OFF_Delay[cur][y][x - distance] = Lowpass(Layers->OFF_Compressed[cur][y][x - distance], Layers->OFF_Compressed[pre][y][x - distance], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                            Layers->OFF_Delay[cur][y + distance][x] = Lowpass(Layers->OFF_Compressed[cur][y + distance][x], Layers->OFF_Compressed[pre][y + distance][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                            Layers->OFF_Delay[cur][y - distance][x] = Lowpass(Layers->OFF_Compressed[cur][y - distance][x], Layers->OFF_Compressed[pre][y - distance][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);


                            // Cov_TripleCorrelation_Bias, bias = 1, using compressed channel.
                            // Rightward : 0   
                            Layers->T4s[cur][y][x][0] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x + distance], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x + distance], bias);
                            Layers->T5s[cur][y][x][0] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x + distance], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x + distance], bias);
                            // Leftward : 1           +  Cov_Triple                                                                                      distance                                                             distance
                            Layers->T4s[cur][y][x][1] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x - distance], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x - distance], bias);
                            Layers->T5s[cur][y][x][1] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x - distance], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x - distance], bias);
                            // Downward : 2           +  Cov_Triple                                                                     
                            Layers->T4s[cur][y][x][2] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y + distance][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y + distance][x], bias);
                            Layers->T5s[cur][y][x][2] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y + distance][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y + distance][x], bias);
                            // Upward : 3             +  Cov_Triple                                                                                   distance                                                             distance
                            Layers->T4s[cur][y][x][3] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y - distance][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y - distance][x], bias);
                            Layers->T5s[cur][y][x][3] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y - distance][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y - distance][x], bias);

                            // using pre_ON/OFF as delay signal instead of low-pass filter : 
                            /*
                            // Rightward : 0
                            Layers->T4s[cur][y][x][0] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x + distance],    Layers->ON_Compressed[!cur][y][x],  Layers->ON_Compressed[!cur][y][x + distance], bias);
                            Layers->T5s[cur][y][x][0] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x + distance], Layers->OFF_Compressed[!cur][y][x], Layers->OFF_Compressed[!cur][y][x + distance], bias);
                            // Leftward : 1           +  Cov_Triple                                                                                      distance               _Compressed !                       _Compressed !
                            Layers->T4s[cur][y][x][1] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x - distance], Layers->ON_Compressed[!cur][y][x], Layers->ON_Compressed[!cur][y][x - distance], bias);
                            Layers->T5s[cur][y][x][1] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x - distance], Layers->OFF_Compressed[!cur][y][x], Layers->OFF_Compressed[!cur][y][x - distance], bias);
                            // Downward : 2           +  Cov_Triple                                                                                                             _Compressed !                       _Compressed !
                            Layers->T4s[cur][y][x][2] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y + distance][x], Layers->ON_Compressed[!cur][y][x], Layers->ON_Compressed[!cur][y + distance][x], bias);
                            Layers->T5s[cur][y][x][2] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y + distance][x], Layers->OFF_Compressed[!cur][y][x], Layers->OFF_Compressed[!cur][y + distance][x], bias);
                            // Upward : 3             +  Cov_Triple                                                                                   distance                  _Compressed !                       _Compressed !
                            Layers->T4s[cur][y][x][3] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y - distance][x], Layers->ON_Compressed[!cur][y][x], Layers->ON_Compressed[!cur][y - distance][x], bias);
                            Layers->T5s[cur][y][x][3] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y - distance][x], Layers->OFF_Compressed[!cur][y][x], Layers->OFF_Compressed[!cur][y - distance][x], bias);
                            */
                        }

                    }else{// #define USE_DYNAMIC_DELAY 0, connected neuron number = 2

                        // EMD with Raw ON/OFF Signal : 
                        /*
                        // Rightward : 0
                        Layers->T4s[cur][y][x][0] = HRCorrelation_Bias (Layers->ON[cur][y][x], Layers->ON[cur][y][x + sd], Layers->ON[pre][y][x], Layers->ON[pre][y][x + sd], bias);
                        Layers->T5s[cur][y][x][0] = HRCorrelation_Bias(Layers->OFF[cur][y][x],Layers->OFF[cur][y][x + sd],Layers->OFF[pre][y][x],Layers->OFF[pre][y][x + sd], bias);
                        // Leftward : 1                                                                                               pre                    pre
                        Layers->T4s[cur][y][x][1] = HRCorrelation_Bias (Layers->ON[cur][y][x], Layers->ON[cur][y][x - sd], Layers->ON[pre][y][x], Layers->ON[pre][y][x - sd], bias);
                        Layers->T5s[cur][y][x][1] = HRCorrelation_Bias(Layers->OFF[cur][y][x],Layers->OFF[cur][y][x - sd],Layers->OFF[pre][y][x],Layers->OFF[pre][y][x - sd], bias);
                        // Downward : 2                                                                                               pre                    pre
                        Layers->T4s[cur][y][x][2] = HRCorrelation_Bias (Layers->ON[cur][y][x], Layers->ON[cur][y + sd][x], Layers->ON[pre][y][x], Layers->ON[pre][y + sd][x], bias);
                        Layers->T5s[cur][y][x][2] = HRCorrelation_Bias(Layers->OFF[cur][y][x],Layers->OFF[cur][y + sd][x],Layers->OFF[pre][y][x],Layers->OFF[pre][y + sd][x], bias);
                        // Upward : 3                                                                                                 pre                    pre
                        Layers->T4s[cur][y][x][3] = HRCorrelation_Bias (Layers->ON[cur][y][x], Layers->ON[cur][y - sd][x], Layers->ON[pre][y][x], Layers->ON[pre][y - sd][x], bias);
                        Layers->T5s[cur][y][x][3] = HRCorrelation_Bias(Layers->OFF[cur][y][x],Layers->OFF[cur][y - sd][x],Layers->OFF[pre][y][x],Layers->OFF[pre][y - sd][x], bias);
                        */

                        // EMD with Compressed Signal : 
                        /*
                        // HRCorrelation_Bias, bias = 1
                        // Rightward : 0
                        Layers->T4s[cur][y][x][0] = HRCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x + sd], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x + sd], bias);
                        Layers->T5s[cur][y][x][0] = HRCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x + sd], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x + sd], bias);
                        // Leftward : 1
                        Layers->T4s[cur][y][x][1] = HRCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x - sd], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x - sd], bias);
                        Layers->T5s[cur][y][x][1] = HRCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x - sd], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x - sd], bias);
                        // Downward : 2
                        Layers->T4s[cur][y][x][2] = HRCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y + sd][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y + sd][x], bias);
                        Layers->T5s[cur][y][x][2] = HRCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y + sd][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y + sd][x], bias);
                        // Upward : 3
                        Layers->T4s[cur][y][x][3] = HRCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y - sd][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y - sd][x], bias);
                        Layers->T5s[cur][y][x][3] = HRCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y - sd][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y - sd][x], bias);
                        */

                        // Div_TripleCorrelation_Bias�� bias = 1
                        /*
                        // Rightward : 0
                        Layers->T4s[cur][y][x][0] = Div_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x + sd], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x + sd], bias);
                        Layers->T5s[cur][y][x][0] = Div_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x + sd], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x + sd], bias);
                        // Leftward : 1             Div_Triple
                        Layers->T4s[cur][y][x][1] = Div_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x - sd], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x - sd], bias);
                        Layers->T5s[cur][y][x][1] = Div_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x - sd], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x - sd], bias);
                        // Downward : 2             Div_Triple
                        Layers->T4s[cur][y][x][2] = Div_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y + sd][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y + sd][x], bias);
                        Layers->T5s[cur][y][x][2] = Div_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y + sd][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y + sd][x], bias);
                        // Upward : 3               Div_Triple
                        Layers->T4s[cur][y][x][3] = Div_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y - sd][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y - sd][x], bias);
                        Layers->T5s[cur][y][x][3] = Div_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y - sd][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y - sd][x], bias);
                        */

                        // Cov_TripleCorrelation_Bias, bias = 1

                        // Rightward : 0
                        Layers->T4s[cur][y][x][0] = Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x + sd], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x + sd], bias);
                        Layers->T5s[cur][y][x][0] = Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x + sd], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x + sd], bias);
                        // Lef tward : 1             Cov_Triple
                        Layers->T4s[cur][y][x][1] = Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x - sd], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x - sd], bias);
                        Layers->T5s[cur][y][x][1] = Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x - sd], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x - sd], bias);
                        // Downward : 2             Cov_Triple
                        Layers->T4s[cur][y][x][2] = Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y + sd][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y + sd][x], bias);
                        Layers->T5s[cur][y][x][2] = Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y + sd][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y + sd][x], bias);
                        // Upward : 3               Cov_Triple
                        Layers->T4s[cur][y][x][3] = Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y - sd][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y - sd][x], bias);
                        Layers->T5s[cur][y][x][3] = Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y - sd][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y - sd][x], bias);
                    }


                    // Lowpass of T4/T5 neurons :
                    Layers->T4s[cur][y][x][0] = Lowpass(Layers->T4s[cur][y][x][0], Layers->T4s[pre][y][x][0], Params->delay_T4);
                    Layers->T5s[cur][y][x][0] = Lowpass(Layers->T5s[cur][y][x][0], Layers->T5s[pre][y][x][0], Params->delay_T5);

                    Layers->T4s[cur][y][x][1] = Lowpass(Layers->T4s[cur][y][x][1], Layers->T4s[pre][y][x][1], Params->delay_T4);
                    Layers->T5s[cur][y][x][1] = Lowpass(Layers->T5s[cur][y][x][1], Layers->T5s[pre][y][x][1], Params->delay_T5);

                    Layers->T4s[cur][y][x][2] = Lowpass(Layers->T4s[cur][y][x][2], Layers->T4s[pre][y][x][2], Params->delay_T4);
                    Layers->T5s[cur][y][x][2] = Lowpass(Layers->T5s[cur][y][x][2], Layers->T5s[pre][y][x][2], Params->delay_T5);

                    Layers->T4s[cur][y][x][3] = Lowpass(Layers->T4s[cur][y][x][3], Layers->T4s[pre][y][x][3], Params->delay_T4);
                    Layers->T5s[cur][y][x][3] = Lowpass(Layers->T5s[cur][y][x][3], Layers->T5s[pre][y][x][3], Params->delay_T5);


                    // Intigration of ON/OFF Motion Signal
                    right = ON_OFF_ChannelSummation(Layers->T4s[cur][y][x][0], Layers->T5s[cur][y][x][0], Params->exp_ON, Params->exp_OFF);
                    left = ON_OFF_ChannelSummation(Layers->T4s[cur][y][x][1], Layers->T5s[cur][y][x][1], Params->exp_ON, Params->exp_OFF);
                    down = ON_OFF_ChannelSummation(Layers->T4s[cur][y][x][2], Layers->T5s[cur][y][x][2], Params->exp_ON, Params->exp_OFF);
                    up = ON_OFF_ChannelSummation(Layers->T4s[cur][y][x][3], Layers->T5s[cur][y][x][3], Params->exp_ON, Params->exp_OFF);

                    // Local Magnitude : 
                    Layers->LPLC2s_mag[y][x] = sqrt(pow(right + left, 2) + pow(up + down, 2));
                    if (Results->MaxMag < Layers->LPLC2s_mag[y][x])
                    {
                        Results->MaxMag = Layers->LPLC2s_mag[y][x];
                        Results->MaxMag_y = y ;
                        Results->MaxMag_x = x ;
                    }

                    // Local Direction : 
                    horizonal = (right > left) ? right : -left;
                    vertical = (up > down) ? up : -down;

                    Layers->LPLC2s_direction[y][x] = atan2(vertical, horizonal) * 180 / M_PI;

                    // Regional integration of visual projection neurons : 
                    if (y <= Results->Attention_Coordinate[0] && x > Results->Attention_Coordinate[1]) {
                        Results->LPLC2_Quadrant_Output[0] += Leaky_ReLU(right - left);
                        Results->LPLC2_Quadrant_Output[0] += Leaky_ReLU(up - down);

                    }
                    else if (y < Results->Attention_Coordinate[0] && x <= Results->Attention_Coordinate[1]) {
                        Results->LPLC2_Quadrant_Output[1] += Leaky_ReLU(left - right);
                        Results->LPLC2_Quadrant_Output[1] += Leaky_ReLU(up - down);

                    }
                    else if (y >= Results->Attention_Coordinate[0] && x < Results->Attention_Coordinate[1]) {
                        Results->LPLC2_Quadrant_Output[2] += Leaky_ReLU(left - right);
                        Results->LPLC2_Quadrant_Output[2] += Leaky_ReLU(down - up);

                    }
                    else {// y > AC[0], x >= AC[1]  
                        Results->LPLC2_Quadrant_Output[3] += Leaky_ReLU(right - left);
                        Results->LPLC2_Quadrant_Output[3] += Leaky_ReLU(down - up);
                    }

                }
            }// Traverse pixels over, we've got 4-quadrants output.
            printf("\nQuadrant output:\n1: %f  2: %f  3: %f  4 : %f\n", Results->LPLC2_Quadrant_Output[0], Results->LPLC2_Quadrant_Output[1], Results->LPLC2_Quadrant_Output[2], Results->LPLC2_Quadrant_Output[3]);

        }
        printf("\nMax Magnitude of Frame %d : %f", *hLPLC2->hFrameCount, Results->MaxMag);



        // Looming response of LPLC2 : 
        // Integrate centrifugal motion information from four quadrants.
        for(int i = 0; i < 4; i++ )
        {
            ThresholdIt(&Results->LPLC2_Quadrant_Output[i],Params->T_lplc2 ) ;

            printf("\n[after Threshold, quadrant output[%d] set to %f ]\n",i , Results->LPLC2_Quadrant_Output[i]) ;
        }
        double OUTPUT = ReLU(Results->LPLC2_Quadrant_Output[0]) * ReLU(Results->LPLC2_Quadrant_Output[1]) * ReLU(Results->LPLC2_Quadrant_Output[2]) * ReLU(Results->LPLC2_Quadrant_Output[3]);
        if (OUTPUT ) {
            Results->LPLC2[*hLPLC2->hFrameCount % LPLC2_ACTIVE_LENGTH] = Results->LPLC2_Quadrant_Output[0]
                                                                       + Results->LPLC2_Quadrant_Output[1]
                                                                       + Results->LPLC2_Quadrant_Output[2]
                                                                       + Results->LPLC2_Quadrant_Output[3];
        }else{
            Results->LPLC2[*hLPLC2->hFrameCount % LPLC2_ACTIVE_LENGTH] = 0;
        }

        // Menbrence Potential : 
        for (int n = 0; n < LPLC2_ACTIVE_LENGTH; n++)
        {
            printf("\npartial_out %d: %f", n, Results->LPLC2[n]);
            Results->LPLC2_Output[*hLPLC2->hFrameCount] += Results->LPLC2[n];
        }
        printf("\n\nLPLC2_out : %f", Results->LPLC2_Output[*hLPLC2->hFrameCount]);
        /*
        if (Results->LPLC2[*hLPLC2->hFrameCount % LPLC2_ACTIVE_LENGTH] != 0) {
            for (int n = 0; n < LPLC2_ACTIVE_LENGTH; n++)
            {
                printf("\npartial_out %d: %f", n, Results->LPLC2[n]);
                Results->LPLC2_Output[*hLPLC2->hFrameCount] += Results->LPLC2[n];
            }
            Results->LPLC2_Output[*hLPLC2->hFrameCount] /= LPLC2_ACTIVE_LENGTH;
        }else{
            Results->LPLC2_Output[*hLPLC2->hFrameCount] /= 2;
        }
        printf("\n\nLPLC2_out : %f", Results->LPLC2_Output[*hLPLC2->hFrameCount]);
        */

    }else{ // MULTI ATTENTION MECHANISM  

        for (int y = (Params->sd_connected) * (Params->sd_step); y < Height - (Params->sd_connected) * (Params->sd_step); y++)
        {
            for (int x = (Params->sd_connected) * (Params->sd_step); x < Width - (Params->sd_connected) * (Params->sd_step); x++)
            {
                // Rightward : 0   
                Layers->T4s[cur][y][x][0] = 0 ; // memset, clear pre_frame remaining signal
                Layers->T5s[cur][y][x][0] = 0 ;
                // Leftward : 1             
                Layers->T4s[cur][y][x][1] = 0 ;
                Layers->T5s[cur][y][x][1] = 0 ;
                // Downward : 2             
                Layers->T4s[cur][y][x][2] = 0 ;
                Layers->T5s[cur][y][x][2] = 0 ;
                // Upward : 3               
                Layers->T4s[cur][y][x][3] = 0 ;
                Layers->T5s[cur][y][x][3] = 0 ;

                for (int distance = Params->sd_step; distance <= (Params->sd_connected) * (Params->sd_step); distance += Params->sd_step)
                {
                    // Dynamic Delay :
                    Layers->ON_Delay[cur][y][x] = Lowpass(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[pre][y][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                    Layers->ON_Delay[cur][y][x + distance] = Lowpass(Layers->ON_Compressed[cur][y][x + distance], Layers->ON_Compressed[pre][y][x + distance], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                    Layers->ON_Delay[cur][y][x - distance] = Lowpass(Layers->ON_Compressed[cur][y][x - distance], Layers->ON_Compressed[pre][y][x - distance], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                    Layers->ON_Delay[cur][y + distance][x] = Lowpass(Layers->ON_Compressed[cur][y + distance][x], Layers->ON_Compressed[pre][y + distance][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                    Layers->ON_Delay[cur][y - distance][x] = Lowpass(Layers->ON_Compressed[cur][y - distance][x], Layers->ON_Compressed[pre][y - distance][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);

                    Layers->OFF_Delay[cur][y][x] = Lowpass(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[pre][y][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                    Layers->OFF_Delay[cur][y][x + distance] = Lowpass(Layers->OFF_Compressed[cur][y][x + distance], Layers->OFF_Compressed[pre][y][x + distance], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                    Layers->OFF_Delay[cur][y][x - distance] = Lowpass(Layers->OFF_Compressed[cur][y][x - distance], Layers->OFF_Compressed[pre][y][x - distance], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                    Layers->OFF_Delay[cur][y + distance][x] = Lowpass(Layers->OFF_Compressed[cur][y + distance][x], Layers->OFF_Compressed[pre][y + distance][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);
                    Layers->OFF_Delay[cur][y - distance][x] = Lowpass(Layers->OFF_Compressed[cur][y - distance][x], Layers->OFF_Compressed[pre][y - distance][x], Params->delay_lp_dynamic[(distance - Params->sd_step) / Params->sd_step]);


                    // Cov_TripleCorrelation_Bias, bias = 1, using compressed channel.
                    // Rightward : 0   
                    Layers->T4s[cur][y][x][0] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x + distance], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x + distance], bias);
                    Layers->T5s[cur][y][x][0] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x + distance], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x + distance], bias);
                    // Leftward : 1           +  Cov_Triple                                                                                      distance                                                             distance
                    Layers->T4s[cur][y][x][1] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y][x - distance], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y][x - distance], bias);
                    Layers->T5s[cur][y][x][1] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y][x - distance], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y][x - distance], bias);
                    // Downward : 2           +  Cov_Triple                                                                     
                    Layers->T4s[cur][y][x][2] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y + distance][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y + distance][x], bias);
                    Layers->T5s[cur][y][x][2] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y + distance][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y + distance][x], bias);
                    // Upward : 3             +  Cov_Triple                                                                                   distance                                                             distance
                    Layers->T4s[cur][y][x][3] += Cov_TripleCorrelation_Bias(Layers->ON_Compressed[cur][y][x], Layers->ON_Compressed[cur][y - distance][x], Layers->ON_Delay[cur][y][x], Layers->ON_Delay[cur][y - distance][x], bias);
                    Layers->T5s[cur][y][x][3] += Cov_TripleCorrelation_Bias(Layers->OFF_Compressed[cur][y][x], Layers->OFF_Compressed[cur][y - distance][x], Layers->OFF_Delay[cur][y][x], Layers->OFF_Delay[cur][y - distance][x], bias);
                }

                // Lowpass of T4/T5 neurons :
                Layers->T4s[cur][y][x][0] = Lowpass(Layers->T4s[cur][y][x][0], Layers->T4s[pre][y][x][0], Params->delay_T4);
                Layers->T5s[cur][y][x][0] = Lowpass(Layers->T5s[cur][y][x][0], Layers->T5s[pre][y][x][0], Params->delay_T5);

                Layers->T4s[cur][y][x][1] = Lowpass(Layers->T4s[cur][y][x][1], Layers->T4s[pre][y][x][1], Params->delay_T4);
                Layers->T5s[cur][y][x][1] = Lowpass(Layers->T5s[cur][y][x][1], Layers->T5s[pre][y][x][1], Params->delay_T5);

                Layers->T4s[cur][y][x][2] = Lowpass(Layers->T4s[cur][y][x][2], Layers->T4s[pre][y][x][2], Params->delay_T4);
                Layers->T5s[cur][y][x][2] = Lowpass(Layers->T5s[cur][y][x][2], Layers->T5s[pre][y][x][2], Params->delay_T5);

                Layers->T4s[cur][y][x][3] = Lowpass(Layers->T4s[cur][y][x][3], Layers->T4s[pre][y][x][3], Params->delay_T4);
                Layers->T5s[cur][y][x][3] = Lowpass(Layers->T5s[cur][y][x][3], Layers->T5s[pre][y][x][3], Params->delay_T5);


                // ON/OFF Intigration 
                right = coeM * ON_OFF_ChannelSummation(Layers->T4s[cur][y][x][0], Layers->T5s[cur][y][x][0], Params->exp_ON, Params->exp_OFF);
                left  = coeM * ON_OFF_ChannelSummation(Layers->T4s[cur][y][x][1], Layers->T5s[cur][y][x][1], Params->exp_ON, Params->exp_OFF);
                down  = coeM * ON_OFF_ChannelSummation(Layers->T4s[cur][y][x][2], Layers->T5s[cur][y][x][2], Params->exp_ON, Params->exp_OFF);
                up    = coeM * ON_OFF_ChannelSummation(Layers->T4s[cur][y][x][3], Layers->T5s[cur][y][x][3], Params->exp_ON, Params->exp_OFF);


                // Local Motion Integration :
                Layers->LM[y][x][0] =  ReLU(right - left , 2.0);
                Layers->LM[y][x][1] =  ReLU(left - right , 2.0);
                Layers->LM[y][x][2] =  ReLU(down - up    , 2.0);
                Layers->LM[y][x][3] =  ReLU(up - down    , 2.0);


                // Local Direction : 
                horizonal = (right > left) ? right : -left;
                vertical = (up > down) ? up : -down;
                Layers->LPLC2s_direction[y][x] = atan2(vertical, horizonal) * 180 / M_PI;


                // Local Magnitude : 
                Layers->LPLC2s_mag[y][x] = sqrt(pow(right + left, 2) + pow(up + down, 2) );


                // Find the MaxMag : 
                if (Results->MaxMag < Layers->LPLC2s_mag[y][x])
                {
                    Results->MaxMag = Layers->LPLC2s_mag[y][x];
                    Results->MaxMag_y = y ;
                    Results->MaxMag_x = x ;
                }





                // Whether current pixel beyond all existed attention fields
                bool BeyondAll = 1 ;
                if( !isEmpty(ExistingAFs) )
                {
                    for(Attention* Current_RF = ExistingAFs->front; Current_RF != nullptr; Current_RF = Current_RF->next )
                    {
                        // search this 80 x 80 field : 
                        if (   Results->MaxMag_y >= Current_RF->Attention_Coordinate[0] - (Params->Judge_Field_size / 2)
                            && Results->MaxMag_y <  Current_RF->Attention_Coordinate[0] + (Params->Judge_Field_size / 2)
                            && Results->MaxMag_x >= Current_RF->Attention_Coordinate[1] - (Params->Judge_Field_size / 2)
                            && Results->MaxMag_x <  Current_RF->Attention_Coordinate[1] + (Params->Judge_Field_size / 2) ){
                            // find it in existed field : no need to establish a new attention field
                            BeyondAll = 0 ; 
                        }else{
                            // does not in current attention field, continue searching other fields
                            continue ;
                        }
                    }

                }

                if(BeyondAll ){
                    if (Results->BEYOND < Layers->LPLC2s_mag[y][x])
                    {           
                        Results->BEYOND = Layers->LPLC2s_mag[y][x];
                        Results->BEYOND_y = y ;
                        Results->BEYOND_x = x ;
                    }
                }


            }
        } // Traverse pixels over

          //       // Find the MaxMag beyond all current existed attention fields : 
          //       for (int y = (Params->sd_connected) * (Params->sd_step); y < Height - (Params->sd_connected) * (Params->sd_step); y++)
          //       {
          //           for (int x = (Params->sd_connected) * (Params->sd_step); x < Width - (Params->sd_connected) * (Params->sd_step); x++)
          //           {
          //
          //               
          //
          //           }
          //       }





          // MULTI ATTENTION SCANNING : 

          // Whether to establish a new attention field or not
        bool NewAttention = 1 ;

        // Whether max point beyond all attention fields && it is expanding toward 4 cardinal directions :
        if(Results->BEYOND == 0){
            // if no max point beyond all attention fields :
            NewAttention = 0 ;
        }
        /*
        else{
        // Judge whether expanding to 4-cardinal directions, if not, attach no attention
        double judge_0 = 0;
        double judge_1 = 0;
        double judge_2 = 0;
        double judge_3 = 0;
        for(int y = Results->BEYOND_y-(Params->Judge_Field_size/2); y < Results->BEYOND_y+(Params->Judge_Field_size/2); y ++ )
        {
        for(int x = Results->BEYOND_x-(Params->Judge_Field_size/2); x < Results->BEYOND_x+(Params->Judge_Field_size/2); x ++ )
        {
        if (y < 0 || y >= Height || x < 0 || x >= Width){
        continue;
        } // avoid out of range

        if (y <= Results->BEYOND_y && x > Results->BEYOND_x) {
        judge_0 += Layers->LM[y][x][0];
        judge_0 += Layers->LM[y][x][3];

        }else if (y < Results->BEYOND_y && Results->BEYOND_x) {
        judge_1 += Layers->LM[y][x][1];
        judge_1 += Layers->LM[y][x][3];

        }else if (y >= Results->BEYOND_y && Results->BEYOND_x) {
        judge_2 += Layers->LM[y][x][1];
        judge_2 += Layers->LM[y][x][2];

        }else {// y > AC[0], x >= AC[1]  
        judge_3 += Layers->LM[y][x][0];
        judge_3 += Layers->LM[y][x][2];   
        }


        }
        }// traverse AFs over, we've got output of 4-quadrants 
        double Expanding = judge_0 * judge_1 * judge_2 * judge_3 ;
        if(Expanding < Params->Judge_Threshold){
        NewAttention = 0 ;
        }
        }
        */


        /*
        // Way 2 : 
        if( !isEmpty(ExistingAFs) )
        {
        for(Attention* Current_RF = ExistingAFs->front; Current_RF != nullptr; Current_RF = Current_RF->next )
        {
        // search this 80 x 80 field : 
        if (   Results->BEYOND_y >= Current_RF->Attention_Coordinate[0] - (Params->RF_size / 2)
        && Results->BEYOND_y <  Current_RF->Attention_Coordinate[0] + (Params->RF_size / 2)
        && Results->BEYOND_x >= Current_RF->Attention_Coordinate[1] - (Params->RF_size / 2)
        && Results->BEYOND_x <  Current_RF->Attention_Coordinate[1] + (Params->RF_size / 2) ){
        // find it in existed field : no need to establish a new attention field
        NewAttention = 0 ; 
        break ;
        }else{
        // does not in current attention field, continue searching other fields
        continue ;
        }
        }
        }
        */


        printf("\nNum of Existing AFs : [%d]", hLPLC2->aModel.Results.ExistingAFs->size ) ;
        printf("\nWhole_Max: [%d, %d], ",Results->MaxMag_y, Results->MaxMag_x );
        printf("Beyond_Max: [%d, %d], %f  Establish New? : [%d]\n",Results->BEYOND_y, Results->BEYOND_x,Results->BEYOND, NewAttention );


        // Arrange an new attention field for new MaxMag point
        if(NewAttention )
        {
            // Initialize new AF
            Attention* NEW = (Attention* )malloc(sizeof(Attention) ) ;
            NEW->next = NULL ;
            NEW->Attention_Coordinate[0] = Results->BEYOND_y ;
            NEW->Attention_Coordinate[1] = Results->BEYOND_x ;
            for(int i = 0; i < LPLC2_ACTIVE_LENGTH; i ++ ){
                NEW->RF_Current_Output[i] = 0 ;
            }
            for(int i = 0; i < FRAME_END-FRAME_BEGIN +1; i ++ ){
                NEW->RF_Output[i] = 0 ;
            }
            NEW->RF_Recent_Output = 0 ;
            NEW->Existing[0] = *hLPLC2->hFrameCount ; 
            NEW->Existing[1] = 120 ; 
            // Who am I ?
            NEW->Number = ++Field_Number;

            // Add new AF to 'Existing AFs' : 
            enFields(ExistingAFs, NEW) ;
            printf("\nNew AF [%d] enField to Existing AFs; ", NEW->Number ) ;


            // Backup new AF to 'Existed AFs'
            Attention* NEW2 = (Attention* )malloc(sizeof(Attention) ) ;
            NEW2->next = NULL ;
            NEW2->Attention_Coordinate[0] = Results->BEYOND_y ;
            NEW2->Attention_Coordinate[1] = Results->BEYOND_x ;
            for(int i = 0; i < LPLC2_ACTIVE_LENGTH; i ++ ){
                NEW2->RF_Current_Output[i] = 0 ;
            }
            for(int i = 0; i < FRAME_END-FRAME_BEGIN +1; i ++ ){
                NEW2->RF_Output[i] = 0 ;
            }
            NEW2->RF_Recent_Output = 0 ;
            NEW2->Existing[0] = *hLPLC2->hFrameCount ; 
            NEW2->Existing[1] = 120 ; 
            // Who am I ?
            NEW2->Number = NEW->Number ;

            // Add new AF to AFs : 
            enFields(ExistedAFs, NEW2) ;            
            printf("New AF [%d] enField to Existed AFs. \n", NEW2->Number ) ;
        }


        // Traverse fields
        // Update all attention fields' information using current frame information
        for(Attention* Field = ExistingAFs->front; Field != NULL; Field = Field->next )
        {
            // Existing AFs : 
            // reset final out put for each frame : 
            Field->RF_Output[*hLPLC2->hFrameCount] = 0;
            Field->RF_Recent_Output = 0 ;
            // make room for attention fields in current frame : 
            Field->RF_Quadrant_Output[0] = 0 ;
            Field->RF_Quadrant_Output[1] = 0 ;
            Field->RF_Quadrant_Output[2] = 0 ;
            Field->RF_Quadrant_Output[3] = 0 ;

            // accumulate pixels in current attention field :
            for(int y = Field->Attention_Coordinate[0]-(Params->RF_size/2); y < Field->Attention_Coordinate[0]+(Params->RF_size/2); y ++ )
            {
                for(int x = Field->Attention_Coordinate[1]-(Params->RF_size/2); x < Field->Attention_Coordinate[1]+(Params->RF_size/2); x ++ )
                {
                    if (y < 0 || y >= Height || x < 0 || x >= Width){
                        continue;
                    } // avoid out of range

                    if (y <= Field->Attention_Coordinate[0] && x > Field->Attention_Coordinate[1]) {
                        Field->RF_Quadrant_Output[0] += Layers->LM[y][x][0];
                        Field->RF_Quadrant_Output[0] += Layers->LM[y][x][3];

                    }else if (y < Field->Attention_Coordinate[0] && x <= Field->Attention_Coordinate[1]) {
                        Field->RF_Quadrant_Output[1] += Layers->LM[y][x][1];
                        Field->RF_Quadrant_Output[1] += Layers->LM[y][x][3];

                    }else if (y >= Field->Attention_Coordinate[0] && x < Field->Attention_Coordinate[1]) {
                        Field->RF_Quadrant_Output[2] += Layers->LM[y][x][1];
                        Field->RF_Quadrant_Output[2] += Layers->LM[y][x][2];

                    }else {// y > AC[0], x >= AC[1]  
                        Field->RF_Quadrant_Output[3] += Layers->LM[y][x][0];
                        Field->RF_Quadrant_Output[3] += Layers->LM[y][x][2];   
                    }


                }
            }// traverse AFs over, we've got output of 4-quadrants of all attention fields in current frame 

            for(int i = 0; i < 4; i++ )
            {
                ThresholdIt(&Results->LPLC2_Quadrant_Output[i],Params->T_lplc2 ) ;

                printf("\n[after Threshold, quadrant output[%d] set to %f ]\n", i, Results->LPLC2_Quadrant_Output[i]) ;
            }
            
            double OUTPUT = Field->RF_Quadrant_Output[0]*Field->RF_Quadrant_Output[1]*Field->RF_Quadrant_Output[2]*Field->RF_Quadrant_Output[3] ; 
            if(OUTPUT ){
                // Sum up : (output more steady values)
                Field->RF_Current_Output[*hLPLC2->hFrameCount % LPLC2_ACTIVE_LENGTH] = Field->RF_Quadrant_Output[0]+Field->RF_Quadrant_Output[1]+Field->RF_Quadrant_Output[2]+Field->RF_Quadrant_Output[3] ;

                // Multiplication : (output varies)
                //Field->RF_Current_Output[*hLPLC2->hFrameCount % LPLC2_ACTIVE_LENGTH] = OUTPUT ;
            }else{
                Field->RF_Current_Output[*hLPLC2->hFrameCount % LPLC2_ACTIVE_LENGTH] = 0 ;
            }
            //printf("\nIn Current Frame:\nField[%d] 4-Q: [%f][%f][%f][%f]\n\n",Field->Number, Field->RF_Quadrant_Output[0],Field->RF_Quadrant_Output[1],Field->RF_Quadrant_Output[2],Field->RF_Quadrant_Output[3] );

            // Menbrence Potential : 

            for (int n = 0; n < LPLC2_ACTIVE_LENGTH; n++)
            {
                //Field->RF_Output[*hLPLC2->hFrameCount] += Field->RF_Current_Output[*hLPLC2->hFrameCount % LPLC2_ACTIVE_LENGTH] ;
                Field->RF_Output[*hLPLC2->hFrameCount] += Field->RF_Current_Output[n] ;
            }
            //Field->RF_Output[*hLPLC2->hFrameCount] /= LPLC2_ACTIVE_LENGTH;

            /*
            if( OUTPUT != 0 ){
            for (int n = 0; n < LPLC2_ACTIVE_LENGTH; n++)
            {
            //printf("\npartial_out NO.%d: %f",  n, Results->LPLC2[n]);
            Field->RF_Output[*hLPLC2->hFrameCount] += Field->RF_Current_Output[*hLPLC2->hFrameCount % LPLC2_ACTIVE_LENGTH] ;
            }
            Field->RF_Output[*hLPLC2->hFrameCount] /= LPLC2_ACTIVE_LENGTH;
            }else {
            for (int n = 0; n < LPLC2_ACTIVE_LENGTH; n++)
            {
            //printf("\npartial_out NO.%d: %f",  n, Results->LPLC2[n]);
            Field->RF_Current_Output[*hLPLC2->hFrameCount % LPLC2_ACTIVE_LENGTH] /= 2 ;
            Field->RF_Output[*hLPLC2->hFrameCount] += Field->RF_Current_Output[*hLPLC2->hFrameCount % LPLC2_ACTIVE_LENGTH] ;
            }
            Field->RF_Output[*hLPLC2->hFrameCount] /= LPLC2_ACTIVE_LENGTH;

            //Field->RF_Output[*hLPLC2->hFrameCount] *= 0.8;
            }
            //printf("\nOut [%d]: %f\n\n",*hLPLC2->hFrameCount, Field->RF_Output[*hLPLC2->hFrameCount] );
            */


            /*
            double pre_out = Field->RF_Current_Output[ (*hLPLC2->hFrameCount + 2) % LPLC2_ACTIVE_LENGTH] ; 

            for (int n = 0; n < LPLC2_ACTIVE_LENGTH; n++)
            {
            //printf("\npartial_out NO.%d: %f",  n, Results->LPLC2[n]);
            Field->RF_Output[*hLPLC2->hFrameCount] += Field->RF_Current_Output[*hLPLC2->hFrameCount % LPLC2_ACTIVE_LENGTH] ;
            }
            Field->RF_Output[*hLPLC2->hFrameCount] /= LPLC2_ACTIVE_LENGTH;


            if(*hLPLC2->hFrameCount > 0){
            if(Field->RF_Output[*hLPLC2->hFrameCount] < 1.001 * Field->RF_Output[*hLPLC2->hFrameCount-1] ){
            Field->RF_Output[*hLPLC2->hFrameCount] = 0 ;
            for (int n = 0; n < LPLC2_ACTIVE_LENGTH; n++)
            {
            Field->RF_Current_Output[*hLPLC2->hFrameCount % LPLC2_ACTIVE_LENGTH] /= 2;
            Field->RF_Output[*hLPLC2->hFrameCount] += Field->RF_Current_Output[*hLPLC2->hFrameCount % LPLC2_ACTIVE_LENGTH] ;
            }
            Field->RF_Output[*hLPLC2->hFrameCount] /= LPLC2_ACTIVE_LENGTH;
            }
            }
            */

            
            // copy information to Existed AFs (Since they are not the identical nodes and linked tables) : 
            for(Attention* Field2 = ExistedAFs->front; Field2 != NULL; Field2 = Field2->next )
            {
                // Find the corresponding node in Existed AFs
                if(Field2->Number == Field->Number )
                {
                    printf("\nWe've found corresponding node in Existed AFs, now update it's value : ") ;
                    printf("\nNode in Existing : [%d], Node in Existed : [%d]; ", Field->Number, Field2->Number ) ;
                    printf("Update it's value to : %f", Field->RF_Output[*hLPLC2->hFrameCount] ) ;

                    Field2->Attention_Coordinate[0] = Field->Attention_Coordinate[0] ;
                    Field2->Attention_Coordinate[1] = Field->Attention_Coordinate[1] ;
                    Field2->Existing[0] = Field->Existing[0] ;
                    Field2->Existing[1] = Field->Existing[1] ;
                    Field2->RF_Output[*hLPLC2->hFrameCount] = Field->RF_Output[*hLPLC2->hFrameCount] ;
                }
            }












            // Abondon current AF at Existing AFs ? 
            // This operation is in Existing AFs.

            //if(*hLPLC2->hFrameCount > Params->d
            if(*hLPLC2->hFrameCount > (Params->d + Field->Existing[0] ) )
            {
                for(int i = 0; i < Params->d; i ++ )
                {
                    Field->RF_Recent_Output += Field->RF_Output[*hLPLC2->hFrameCount-i] ;
                }

                if(Field->RF_Recent_Output < Params->T_af && ExistingAFs->size > 1 )
                {   
                    // This way is fine ...
                    Field->Existing[1] = *hLPLC2->hFrameCount ;
                    Field = removeField(ExistingAFs, Field, *hLPLC2->hFrameCount) ;

                    for(Attention* Field2 = ExistedAFs->front; Field2 != NULL; Field2 = Field2->next )
                    {
                        // Find the corresponding node in Existed AFs
                        if(Field2->Number == Field->Number )
                        {
                            Field2->Existing[1] = *hLPLC2->hFrameCount ;
                            break ;
                        }
                    }
                    /* 
                    // Using this way will lead to interuption at ~Field 10 ...  
                    Attention* next = Field->next ;
                    void_removeField(ExistingAFs, Field, *hLPLC2->hFrameCount) ;
                    Field = next ;
                    */
                }
            }

        }
        printf("\n\nCurrent Existing AFs : ") ;
        traverseFieldSet(ExistingAFs->front, hLPLC2);
        //traverseFieldSet(ExistedAFs->front, hLPLC2);


    }

    printf("\n\nFrame %d Calculated Over...\n", *hLPLC2->hFrameCount);
    return 1;
}



// Supervise the Program : 

// for output vedio
void SaveFrameAsImage(cv::Mat& image, LPLC2_pControlTypedef* hLPLC2, int Height, int Width, int Frame, int direction)
{
    for (int i = 0; i < Height; i++)
    {
        for (int j = 0; j < Width; j++)
        {
            float value = hLPLC2->aModel.Layers.T5s[hLPLC2->currentDiffImage][i][j][direction];

            image.at<uchar>(i, j) = static_cast<uchar>(value * 255);
        }
    }

    // Save : 
    //std::string filename = "frame_" + std::to_string(Frame) + ".png";
    //cv::imwrite(filename, image);
}

// for output vedio
void markAF(cv::Mat& image, LPLC2_pControlTypedef* hLPLC2, int Height, int Width, int Frame, int direction)
{
    for (int i = 0; i < Height; i++)
    {
        for (int j = 0; j < Width; j++)
        {
            float value = hLPLC2->aModel.Layers.T5s[hLPLC2->currentDiffImage][i][j][direction];

            image.at<uchar>(i, j) = static_cast<uchar>(value * 255);
        }
    }

    // Save : 
    //std::string filename = "frame_" + std::to_string(Frame) + ".png";
    //cv::imwrite(filename, image);
}

// FAVOR
void ShowFrame(const std::vector<std::vector<std::vector<uint8_t>>>& Vedio, \
    LPLC2_pControlTypedef hLPLC2, int frame_index, int height, int width, double scale_factor, int time)
{
    if (frame_index < 0 || frame_index >= Vedio[0][0].size()) {
        std::cout << "Frame index out of range!" << std::endl;
        return;
    }
    int sd = hLPLC2.aModel.Params.sd;
    bool cur = hLPLC2.currentDiffImage;
    bool pre = !cur;
    // simplize ptr passing : 
    LPLC2struct_Params* Params = &hLPLC2.aModel.Params;
    LPLC2struct_Layers* Layers = &hLPLC2.aModel.Layers;
    LPLC2struct_Results* Results = &hLPLC2.aModel.Results;

    /*
    unsignal   unsignal    signal      signal      float       double

    1 channel:  CV_8UC1    CV_16UC1    CV_16SC1    CV_32SC1    CV_32FC1    CV_64FC1
    3 channel:  CV_8UC3    CV_16UC3    CV_16SC3    CV_32SC3    CV_32FC3    CV_64FC3
    4 channel:  CV_8UC4    CV_16UC4    CV_16SC4    CV_32SC4    CV_32FC4    CV_64FC4
    */
    cv::Mat frame(height, width, CV_8UC1); // uint8_t


    // Input
    for (int i = 0; i < height; i++) {
        //printf("\nraw%d ", i);
        for (int j = 0; j < width; j++) {

            // 0 --> black, 255 --> white


            // Kernel : 
            //frame.at<uint8_t>(i, j) = 1500 * abs(hLPLC2.aModel.Layers.Kernel_GB_Exc[i][j]);
            //frame.at<uint8_t>(i, j) = 1500 * abs(hLPLC2.aModel.Layers.Kernel_GB_Inh[i][j]);


            // Layers : 

            // Diff_Img : 
            //if (hLPLC2.aModel.Layers.Diff_Img[0][i][j] > 0)
            //printf("%d ", hLPLC2.aModel.Layers.Diff_Img[0][i][j]); // watch
            //frame.at<uint8_t>(i, j) =   abs(hLPLC2.aModel.Layers.Diff_Img[cur][i][j]);

            // Summation
            // S_layer, MEANINGLESS: S_Layer contains positive and negative vale, but uint8_t only shows positive.
            //if(hLPLC2.aModel.Layers.Summation[i][j] < 0 )
            //printf("%f ", hLPLC2.aModel.Layers.Summation[i][j]);
            //frame.at<uint8_t>(i, j) = (hLPLC2.aModel.Layers.Summation[i][j]); 

            // ON
            //frame.at<uint8_t>(i, j) = 255 * hLPLC2.aModel.Layers.ON[0][i][j]; // ON_Layer, not obvious-->[ball stimuli\\black app1.mp4]
            // OFF
            //if(hLPLC2.aModel.Layers.OFF[time][i][j] > 1)
            //printf("\nOFF_Layer:%f ", 50 * hLPLC2.aModel.Layers.OFF[time][i][j]);
            //frame.at<uint8_t>(i, j) = 50 * hLPLC2.aModel.Layers.OFF[time][i][j]; // OFF_Layer, obvious-->[ball stimuli\\black app1.mp4]

            // Compressed ON/OFF
            //if (hLPLC2.aModel.Layers.OFF_Compressed[1][i][j] > 0.5)
            //printf("%f  ", hLPLC2.aModel.Layers.OFF_Compressed[1][i][j]);
//            frame.at<uint8_t>(i, j) = 255 * hLPLC2.aModel.Layers.OFF_Compressed[cur][i][j];
            //frame.at<uint8_t>(i, j) = 255 * hLPLC2.aModel.Layers.ON_Compressed[cur][i][j];

            // Delayed ON/OFF
            //frame.at<uint8_t>(i, j) =  255 * hLPLC2.aModel.Layers.OFF_Delay[time][i][j]; 

            //if (hLPLC2.aModel.Layers.OFF_Delay[time][i][j] > 0.3)
            //printf("delay: %f\n ", hLPLC2.aModel.Layers.OFF_Delay[time][i][j]); //

            // T4
            //frame.at<uint8_t>(i, j) = 100 * hLPLC2.aModel.Layers.T4s[cur][i][j][0];

            // T5
            //frame.at<uint8_t>(i, j) = 1000* hLPLC2.aModel.Layers.T5s[time][i][j][0]; // rightward
            //if (hLPLC2.aModel.Layers.T5s[time][i][j][0] > 0)
            //  printf("%f ",1000 * hLPLC2.aModel.Layers.T5s[time][i][j][0]);
            //frame.at<uint8_t>(i, j) = 255 * hLPLC2.aModel.Layers.T5s[time][i][j][1];
            //frame.at<uint8_t>(i, j) = 255 * hLPLC2.aModel.Layers.T5s[time][i][j][2]; 
            //frame.at<uint8_t>(i, j) = 255 * hLPLC2.aModel.Layers.T5s[time][i][j][3];

            // Magnitude 
            //frame.at<uint8_t>(i, j) =  255*(Layers->LPLC2s_mag[i][j]/Results->MaxMag );
            //if (hLPLC2.aModel.Layers.LPLC2s_mag[i][j] > 0)
            //printf("%f \n", hLPLC2.aModel.Layers.LPLC2s_mag[i][j]);

            // Local Motion  LM
            frame.at<uint8_t>(i, j) =  100*(Layers->LM[i][j][3] );

        }
    }


    // Zoom in : 
    cv::Mat frame_resized;
    cv::resize(frame, frame_resized, cv::Size(), scale_factor, scale_factor, cv::INTER_LINEAR); // scale_factor

    cv::imshow("Frame " + std::to_string(frame_index), frame_resized);
    cv::waitKey(0);
}



#pragma endregion Founction_End


int main(void)
{
#pragma region Input vedio into 3-demention matrix : Vedio[72,99,90], uint8_t, 0~255. Initialze output vedio

    cv::VideoCapture cap(VEDIO_PATH);
    if (!cap.isOpened()) {
        std::cout << "Open video failed !" << std::endl;
        return -1;
    }

    // Video Property : 
    Height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    Width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    Total_Frame = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
    FPS = cap.get(cv::CAP_PROP_FPS);

    // 3-Demention Matrix of Input Vedio : (height, width, frame_count)
    std::vector<std::vector<std::vector<uint8_t>>> Vedio(Height , std::vector<std::vector<uint8_t>>(Width , std::vector<uint8_t>(Total_Frame, 0 ) ) ) ;

    int frame_idx = 0;
    cv::Mat frame, gray_frame;
    while (cap.read(frame)) 
    {
        cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY); // B, G, R --> gray-sacle
        // Traverse Pixel : 
        for (int i = 0; i < Height; ++i) 
        {
            for (int j = 0; j < Width; ++j) 
            {
                Vedio[i][j][frame_idx] = gray_frame.at<uint8_t>(i, j);
            }
        }
        frame_idx++;
    }

    cap.release();
    std::cout << "\n\n" << std::endl;
    std::cout<<"Video Information:\nFrame in total: "<<Total_Frame<<"   FPS: "<<FPS<<"   Height: "<<Height<<"   Width: "<<Width<<std::endl;
    std::cout << "\n\n\n\n\n\n\n\n\n\n" << std::endl;

    // Output Vedio Defination : 
    cv::VideoWriter video(OUTPUT_VEDIO_NAME, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, cv::Size(Width, Height), false);


#pragma endregion



    // Declaration of hLPLC2 : 
    LPLC2_pControlTypedef* hLPLC2;
    hLPLC2 = (LPLC2_pControlTypedef*)malloc(sizeof(LPLC2_pControlTypedef));
    // simplize ptr passing : 
    LPLC2struct_Params* Params = &hLPLC2->aModel.Params;
    LPLC2struct_Layers* Layers = &hLPLC2->aModel.Layers;
    LPLC2struct_Results* Results = &hLPLC2->aModel.Results;

    LPLC2_Init(hLPLC2);

    // Now we begin to TRAVERSE FRAMES from 0 to end :  
    for (std::uint32_t Frame = FRAME_BEGIN; Frame < FRAME_END; Frame++)
    {
        hLPLC2->hFrameCount = &Frame; // 
        hLPLC2->currentImage = Frame; // In Colias: 0, 1, 2 ; In computer : 1, 2, ..., 100, ... 
        hLPLC2->currentDiffImage = Frame % 2; // 0 or 1
        hLPLC2->previousDiffImage = !hLPLC2->currentDiffImage; // 1 or 0
        printf("\nNow we have frame %d :\n", Frame );




        LPLC2_Calculating(hLPLC2, Vedio); 






//        ShowFrame(Vedio, *hLPLC2, Frame, Height, Width, 2, hLPLC2->currentDiffImage);


        
        // Storage Current Frame : 
        cv::Mat image(Height, Width, CV_8UC1); // uint8_t

        SaveFrameAsImage(image, hLPLC2, Height, Width, Frame, 1);
        
        video.write(image);
        std::cout << "Frame " << Frame << " Input into Output Vedio.\n\n\n" << std::endl;
        


    }// Traverse Frames Over





     // Output result to .txt file : 

    if(USE_MULTI_ATTENTION_MECHANISM){

        printf("\n\nCurrent Existing AFs : \n") ;
        traverseFieldSet(Results->ExistingAFs->front, hLPLC2);
        writeToText_Existing_AFs(Results->ExistingAFs ) ;
        
        printf("\n\nExisted AFs before filt : \n") ;
        traverseFieldSet(Results->ExistedAFs->front, hLPLC2);
        writeToText_Existed_AFs(Results->ExistedAFs ) ;


        // Filt Fatal AFs From Existed AFs : 
        filtFatalAFsFromExisted(hLPLC2, Results->ExistedAFs, Results->FatalAFs) ;
        //printf("\nExisted AFs after filt (be destroyed): \n") ;
        //traverseFieldSet(Results->ExistedAFs->front, hLPLC2);


        printf("\n\nFatal AFs : \n") ;
        traverseFieldSet(Results->FatalAFs->front, hLPLC2);
        writeToText_Fatal_AFs(Results->FatalAFs ) ;

    }else{
        writeToText(*hLPLC2 ) ;
    }


    video.release();
    printf("\n\n\n\nInput Vedio Operated Over: %s \n\nOutput Vedio: %s\n\n\n\n", VEDIO_PATH, OUTPUT_VEDIO_NAME);



    // Free Malloc : 
    Free3DLayer(&Layers->Diff_Img, 2, Height);
    Free2DLayer(&Layers->Summation, Height); // Input first demention length
    Free2DLayer(&Layers->Kernel_GB_Exc, Params->width_exc);
    Free2DLayer(&Layers->Kernel_GB_Inh, Params->width_inh);

    Free3DLayer(&Layers->ON, 2, Height); // Input first and second demention length
    Free3DLayer(&Layers->OFF, 2, Height);

    Free2DLayer(&Layers->Kernel_Contrast, Params->width_cn);
    Free3DLayer(&Layers->ON_Compressed, 2, Height);
    Free3DLayer(&Layers->OFF_Compressed, 2, Height);
    Free3DLayer(&Layers->ON_Delay, 2, Height);
    Free3DLayer(&Layers->OFF_Delay, 2, Height);

    Free4DLayer(&Layers->T4s, 2, Height, Width);
    Free4DLayer(&Layers->T5s, 2, Height, Width);
    Free3DLayer(&Layers->LM, Height, Width);

    Free2DLayer(&Layers->LPLC2s_mag, Height);
    Free2DLayer(&Layers->LPLC2s_direction, Height);


    // Since these two linked table sharing some identical nodes, free the bigger one is enough. Here we free "ExistedAFs".
    //freeFieldSet(Results->ExistingAFs ) ;
    //printf("\n\nFree Existing AFs Over ! \n\n") ;
    freeFieldSet(Results->ExistedAFs) ;
    printf("\nFree ALL_Fields Over ! \n\n\n") ;

    return 0;
}