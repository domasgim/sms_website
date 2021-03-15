<?php

namespace App;

use Illuminate\Database\Eloquent\Model;

class SentMessage extends Model
{
    protected $fillable = ['sender', 'recipient', 'content'];
    protected $casts = [
        'created_at' => 'datetime:Y-m-d H:i:s',
    ];
}
