#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <algorithm>

using namespace std;

int p;
int rows, cols, drones, deadline, load, products;
vector<int> prod_weights;
ofstream of;

struct location
{
    int row, column;
};
struct warehouse
{
    int row, col;
    vector<int> available_prods;
    warehouse()
    {
        row = col = 0;
    }
    warehouse(int row, int col, vector<int>& available_prods)
    {
        this->row = row;
        this->col = col;
        this->available_prods = available_prods;
    }
    
    bool is_empty()
    {
        for (auto x: available_prods)
            if (x > 0)
                return false;
        return true;
    }
    
    bool take_item(int item) // returns false is item not available
    {
        if (available_prods[item] > 0)
        {
            available_prods[item]--;
            return true;
        }
        else
            return false;
    }
};

vector<warehouse> wares;
struct order
{
    int row, col;
    vector<int> ordered_prods;
    order()
    {
        row = col = 0;
    }
    order(int row, int col, vector<int>& available_prods)
    {
        this->row = row;
        this->col = col;
        this->ordered_prods = ordered_prods;
    }
    
    bool is_empty()
    {
        for (auto x: ordered_prods)
            if (x > 0)
                return false;
        return true;
    }
    
    bool recieve_item(int item_id) //returns false if item was already delivered/never ordered
    {
        if (ordered_prods[item_id] > 0)
        {
            ordered_prods[item_id]--;
            return true;
        }
        return false;
    }
};

vector<order> orders;

struct drone
{
    int id;
    int row, col;
    vector<int> carried_items;
    vector<int> excepted_orders;
    vector<int> excepted_warehouses;
    int warehouse_id, droppoint_id;
    
    drone()
    {
        id = row = col = 0;
        warehouse_id = droppoint_id = 0;
        carried_items = vector<int>(p);
        for (auto x: carried_items)
            x = 0;
    }
    
    drone(int id)
    {
        this->id = id;
        row = col = 0;
        warehouse_id = droppoint_id = 0;
        carried_items = vector<int>(p);
        for (auto x: carried_items)
            x = 0;
    }
    
    int current_weight()
    {
        int sum = 0;
        for (int i = 0; i < carried_items.size(); ++i)
        {
            sum += prod_weights[i] * carried_items[i];
        }
        return sum;
    }
    
    int find_nearest_warehouse()
    {
        warehouse w = wares[0];
        warehouse_id = 0;
        double min_distance = sqrt((double)(row - w.row) * (row - w.row) + (col - w.col) * (col - w.col));
        for (int i = 1; i < wares.size(); ++i)
        {
            double distance = sqrt((double)(row - wares[i].row) * (row - wares[i].row) + (col - wares[i].col) * (col - wares[i].col));
            
            if (distance < min_distance && !wares[i].is_empty() && find(excepted_warehouses.begin(), excepted_warehouses.end(), i) == excepted_warehouses.end())
            {
                w = wares[i];
                warehouse_id = i;
                min_distance = distance;
            }
        }
        
        return warehouse_id;
        
    }
    
    int find_nearest_droppoint()
    {
        order o = orders[0];
        int order_id = 0;
        double min_distance = sqrt((double)(row - o.row) * (row - o.row) + (col - o.col) * (col - o.col));
        for (int i = 1; i < orders.size(); ++i)
        {
            double distance = sqrt((double)(row - orders[i].row) * (row - orders[i].row) + (col - orders[i].col) * (col - orders[i].col));
            
            if (distance < min_distance && find(excepted_orders.begin(), excepted_orders.end(), i) == excepted_orders.end())
            {
                o = orders[i];
                order_id = i;
                min_distance = distance;
            }
        }
        
        return droppoint_id;
        
    }
    
    void loadup()
    {
        bool found_warehouse = false;
        bool found_droppoint = false;
        while (!found_warehouse)
        {
            warehouse_id = find_nearest_warehouse();
            cout << warehouse_id << endl;
            row = wares[warehouse_id].row;
            col = wares[warehouse_id].col;
            while (!found_droppoint)
            {
                droppoint_id = find_nearest_droppoint();
                for (int i = 0; i < min(wares[warehouse_id].available_prods.size(), orders[droppoint_id].ordered_prods.size()); ++i)
                    if (wares[warehouse_id].available_prods[i] > 0 && orders[droppoint_id].ordered_prods[i] > 0)
                        found_droppoint = true;
                
            }
            
            found_warehouse = found_droppoint;
        }
        
        if (found_warehouse && found_droppoint)
        {
            for (int i = 0; i < min(wares[warehouse_id].available_prods.size(), orders[droppoint_id].ordered_prods.size()); ++i)
                if (wares[warehouse_id].available_prods[i] > 0 && orders[droppoint_id].ordered_prods[i] > 0)
                    if (wares[warehouse_id].take_item(i))
                        if (!store_item(i))
                            break;
                        else
                            of << id << " L " << warehouse_id << ' ' << i << ' ' << 1 << endl;
            
        }
    }
    
    bool store_item(int item_id)
    {
        if (current_weight() + prod_weights[item_id] > load)
            return false;
        this->carried_items[item_id]++;
        return true;
    }
    
    void deliver()
    {
        for (int i = 0; i < this->carried_items.size(); ++i)
            if (orders[droppoint_id].recieve_item(i))
            {
                carried_items[i]--;
                of << id << " D " << droppoint_id << ' ' << i << ' ' << 1 << endl;
            }
    }
    
};

int main(int argc, char ** argv)
{
    ifstream f;
    f.open(argv[1]);
    of.open(argv[2]);
    f >> rows >> cols >> drones >> deadline >> load;
    
    f >> p;
    prod_weights.reserve(p);
    for (int i = 0; i < p; ++i)
    {
        f >> prod_weights[i];
    }
    
    int w;
    f >> w;
    wares.reserve(w);
    for (int i = 0; i < w; ++i)
    {
        int row, col;
        f >> row >> col;
        vector<int> products(p);
        for (int j = 0; j < p; ++j)
        {
            int n;
            f >> n;
            cout << n << ' ';
            products[j] = n;
        }
        cout << endl;
        
        wares[i] = warehouse(row, col, products);
        cout << row << ' ' << col << ' ' << endl;
    }
    
    int c;
    f >> c;
    orders.reserve(c);
    for (int i = 0; i < c; ++i)
    {
        int rows, cols;
        f >> rows >> cols;
        int l;
        f >> l;
        vector<int> products(p);
        for (auto x: products)
            x = 0;
        
        for (int j = 0; j < l; ++j)
        {
            int n;
            f >> n;
            products[n]++;
            cout << n << ' ';
        }
        cout << endl;
        
        orders[i] = order(rows, cols, products);
        
    }
    
	vector<drone> ds(drones);
    
    while (deadline--)
    {
        for (auto x: ds)
        {
            x.loadup();
            x.deliver();
            of.flush();
        }
    }
    
    of.close();
    return 0;
}
