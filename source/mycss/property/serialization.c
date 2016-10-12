/*
 Copyright (C) 2016 Alexander Borisov
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 
 Author: lex.borisov@gmail.com (Alexander Borisov)
*/

#include "mycss/property/serialization.h"
#include "mycss/property/resources_name.h"

void mycss_property_serialization_type_name(mycss_property_type_t prop_type, mycss_callback_serialization_f callback, void* context)
{
    if(prop_type >= MyCSS_PROPERTY_TYPE_LAST_ENTRY)
        return;
    
    const char* name = mycss_property_index_type_name[prop_type];
    callback(name, strlen(name), context);
}

void mycss_property_serialization_value(unsigned int value_type, void* value, mycss_callback_serialization_f callback, void* context)
{
    switch (value_type) {
        case MyCSS_PROPERTY_VALUE_LENGTH:
            mycss_values_serialization_length(value, callback, context);
            break;
            
        case MyCSS_PROPERTY_VALUE_PERCENTAGE:
            mycss_values_serialization_percentage(value, callback, context);
            break;
            
        default:
        {
            if(value_type >= MyCSS_PROPERTY_VALUE_LAST_ENTRY)
                break;
            
            const char* text_value = mycss_property_index_type_value[value_type];
            callback(text_value, strlen(text_value), context);
            
            break;
        }
    }
}

