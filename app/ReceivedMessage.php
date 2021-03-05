<?php

namespace App;

use Illuminate\Database\Eloquent\Model;

class ReceivedMessage extends Model
{
    protected $fillable = ['sender', 'recipient', 'content', 'read'];
    protected $casts = [
        'created_at' => 'datetime:Y-m-d H:i:s',
    ];
}
