#include "texture.hpp"

#include <stdio.h>
#include <string>

namespace plane_render {

namespace {

#define  MAXLINE 1024

/* read_ppm_header:
 * Read the header contents of a PPM (Portable Pix[el] Map) file.
 * An ASCII PPM image file follows the format:
 * P3
 * <X> <Y> 
 * <colors>
 * <R1> <G1> <B1> ... <RMAX> <GMAX> <BMAX>
 * A binary PPM image file uses P6 instead of P3 and 
 * the data values are represented in binary. 
 * NOTE1: Comment lines start with '#'.
 # NOTE2: < > denote integer values (in decimal).
 */
void read_ppm_header(FILE *f, int *img_xdim, int *img_ydim, int *img_colors, int *is_ascii)
{
  int flag=0;
  int x_val, y_val, maxcolors_val;
  unsigned int i;
  char magic[MAXLINE];
  char line[MAXLINE];
  int count=0;
 
  /* Read the PPM file header. */
  while (fgets(line, MAXLINE, f) != NULL) {
    flag = 0;
    for (i = 0; i < strlen(line); i++) {
      if (isgraph(line[i]) && (flag == 0)) {
        if ((line[i] == '#') && (flag == 0)) {
          flag = 1;
        }
      }
    }
    if (flag == 0) {
      if (count == 0) {
        count += sscanf(line, "%s %d %d %d", magic, &x_val, &y_val, &maxcolors_val); 
      } else if (count == 1) {
        count += sscanf(line, "%d %d %d", &x_val, &y_val, &maxcolors_val);
      } else if (count == 2) {
        count += sscanf(line, "%d %d", &y_val, &maxcolors_val);
      } else if (count == 3) {
        count += sscanf(line, "%d", &maxcolors_val);
      }
    }
    if (count == 4) {
      break;
    }
  }

  if (strcmp(magic, "P3") == 0) {
    *is_ascii = 1;
  } else if (strcmp(magic, "P6") == 0) {
    *is_ascii = 0;
  } else {
    fprintf(stderr, "Error: Input file not in PPM format!\n");
    exit(1);
  }

  fprintf(stderr, "Info: magic=%s, x_val=%d, y_val=%d, maxcolors_val=%d\n",
    magic, x_val, y_val, maxcolors_val);
  *img_xdim   = x_val;
  *img_ydim   = y_val;
  *img_colors = maxcolors_val;
}

/* read_ppm_data:
 * Read the data contents of a PPM (portable pix map) file.
 */
void read_ppm_data(FILE *f, int *img_in, int is_ascii)
{
  int i=0, c;
  int r_val, g_val, b_val;
    
  /* Read the rest of the PPM file. */
  while ((c = fgetc(f)) != EOF) {
    ungetc(c, f);
    if (is_ascii == 1) {
      if (fscanf(f, "%d %d %d", &r_val, &g_val, &b_val) != 3) return;
    } else {
      r_val = fgetc(f);
      g_val = fgetc(f);
      b_val = fgetc(f);
    }
    img_in[i++] = r_val;
    img_in[i++] = g_val;
    img_in[i++] = b_val;
  }
  fclose(f);
}

} //namespace

void Texture::Load(const std::string& fname)
{
    FILE* ppm_file = fopen(fname.c_str(), "r");
    CHECK_ALWAYS(ppm_file);

    int width = 0;
    int height = 0;
    int img_colors = 0;
    int is_ascii = 0;
    read_ppm_header(ppm_file, &width, &height, &img_colors, &is_ascii);
    
    CHECK_ALWAYS((width == height) && (width % StorageBlockSide == 0));
    side_ = width;
    blocks_by_side_ = side_ / StorageBlockSide;

    CHECK_ALWAYS(img_colors == 255);
    CHECK_ALWAYS(is_ascii != 1); // binary file
    int* texture_tmp = new int[side_*side_*3];
    read_ppm_data(ppm_file, texture_tmp, is_ascii);

    texture_ = new Color[side_*side_];
    for (size_t block_id = 0; block_id < (side_*side_) / StorageBlockSize; block_id++)
     for (size_t pos_in_block = 0; pos_in_block < StorageBlockSize; pos_in_block++)
     {
         Point2D<size_t> pos = { StorageBlockSide*(block_id % blocks_by_side_) + (pos_in_block % StorageBlockSide),
                                 StorageBlockSide*(block_id / blocks_by_side_) + (pos_in_block / StorageBlockSide) };
         size_t position = (pos.y*side_ + pos.x)*3;
         texture_[block_id*StorageBlockSize + pos_in_block].A = 0;
         texture_[block_id*StorageBlockSize + pos_in_block].R = static_cast<Color::ColorElement>(texture_tmp[position]);
         texture_[block_id*StorageBlockSize + pos_in_block].G = static_cast<Color::ColorElement>(texture_tmp[position+1]);
         texture_[block_id*StorageBlockSize + pos_in_block].B = static_cast<Color::ColorElement>(texture_tmp[position+2]);
     }

    delete[] texture_tmp;

    // closed in library
    //fclose(ppm_file); 
}

Texture::~Texture()
{
    delete[] texture_;
}

} // namespace plane_render